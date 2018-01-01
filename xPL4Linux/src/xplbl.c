#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <dlfcn.h>
#include <semaphore.h>

#include "xPL.h"

#include "code_ir_rf.h"
#include "xpld.h"
#include "xplbl.h"
#include "log.h"

#define BL_HEAD_LENGTH	0x38
#define BL_OFF_MODE		0x26
#define BL_MODE_DISCOVERY_REQ	0x06
#define BL_MODE_DISCOVERY_RESP	0x07
#define BL_MODE_AUTH_REQ		0x65
#define BL_MODE_AUTH_RESP		0xe9
#define BL_MODE_CMD_REQ		0x6a
#define BL_MODE_CMD_RESP		0xee

#define BL_CMD_TEMP		0x01
#define BL_CMD_SEND		0x02
#define BL_CMD_LEARN		0x03
#define BL_CMD_LEARN_DATA	0x04

#define DISCOVERY_TIMEOUT	2

static const DEV_NAME dev_names[] = {
	{ 0x272a, "RM2_Pro_Plus" }
};

BL_DEVICE	bl_dev[8];
int			bl_dev_count = 0;

int				bl_socket = 0;
struct sockaddr_in	local_addr;

typedef int (*ptr_aes_crypt_cbc)(void*, int, size_t, void*, void*, void*);
typedef int (*ptr_aes_setkey)(void*, void*, unsigned int);

void *cryptolib = 0;
ptr_aes_crypt_cbc func_aes_crypt_cbc = 0;
ptr_aes_setkey func_aes_setkey_enc = 0;
ptr_aes_setkey func_aes_setkey_dec = 0;

static const int dev_names_count = sizeof(dev_names) / sizeof(dev_names[0]);

uint8_t	bl_master_key[] = { 0x09, 0x76, 0x28, 0x34, 0x3f, 0xe9, 0x9e, 0x23, 0x76, 0x5c, 0x15, 0x13, 0xac, 0xcf, 0x8b, 0x02 };
uint8_t	bl_master_iv[]  = { 0x56, 0x2e, 0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58 };

void get_dev_name(uint16_t id, char name[16])
{
	const DEV_NAME	*dev_name;

	for (dev_name = dev_names; dev_name < dev_names + dev_names_count; dev_name++)
		if (dev_name->id == id) {
			strcpy(name, dev_name->name);
			return;
		}

	sprintf(name, "0x%04x", id);
}

void device_info(const BL_DEVICE *dev, char* info)
{
	_log(LOG_NOTICE, "Device [%s %s], %d.%d.%d.%d (%02x:%02x:%02x:%02x:%02x:%02x) %s",
		dev->devtype_name,
		dev->dev_name,
		dev->addr.sin_addr.s_addr >> 24,
		(dev->addr.sin_addr.s_addr >> 16) &0xff,
		(dev->addr.sin_addr.s_addr >> 8) &0xff,
		dev->addr.sin_addr.s_addr &0xff,
		dev->mac[5], dev->mac[4], dev->mac[3],
		dev->mac[2], dev->mac[1], dev->mac[0],
		info);
}

BL_DEVICE* bl_find_device_by_addr(struct sockaddr_in *addr)
{
	BL_DEVICE	*dev;

	for (dev = bl_dev; dev < bl_dev + bl_dev_count; dev++) {
		if (0 == memcmp(&(dev->addr.sin_addr), &(addr->sin_addr), sizeof(struct in_addr)))
			return dev;
	}
	return 0;
}

void bl_send_cmd(BL_DEVICE *dev, uint8_t *buffer, int in_len)
{
	struct timespec	timeout ;
	
	uint8_t		aes_ctx[300];;
	uint8_t		iv[16];
	uint16_t		checksum;
	int			i;
	int			enc_len;

	/* Set header pattern */
	buffer[0x00] = 0x5a;
	buffer[0x01] = 0xa5;
	buffer[0x02] = 0xaa;
	buffer[0x03] = 0x55;
	buffer[0x04] = 0x5a;
	buffer[0x05] = 0xa5;
	buffer[0x06] = 0xaa;
	buffer[0x07] = 0x55;
	
	buffer[0x24] = 0x2a;
	buffer[0x25] = 0x27;
	
	/* counter */
	bl_dev->count++;
	buffer[0x28] = dev->count & 0xff;
	buffer[0x29] = dev->count >> 8;

	memcpy(buffer + 0x2a, dev->mac, 6);
	memcpy(buffer + 0x30, dev->id, 4);

	/* header checksum */
	for (checksum = 0xbeaf, i = BL_HEAD_LENGTH; i < in_len; i++)
		checksum += buffer[i];

	buffer[0x34] = checksum & 0xff;
	buffer[0x35] = checksum >> 8;

	enc_len = (in_len - BL_HEAD_LENGTH + (1 << 4) - 1) & ~((1 << 4) - 1);
	memcpy(iv, bl_master_iv, sizeof(bl_master_iv));
	func_aes_setkey_enc(&aes_ctx, dev->key, 128);
	func_aes_crypt_cbc(&aes_ctx, 1, enc_len, iv, buffer + BL_HEAD_LENGTH, buffer + BL_HEAD_LENGTH);

	/* payload checksum */
	for (checksum = 0xbeaf, i = 0; i < BL_HEAD_LENGTH + enc_len; i++)
		checksum += buffer[i];

	buffer[0x20] = checksum & 0xff;
	buffer[0x21] = checksum >> 8;

	clock_gettime(CLOCK_REALTIME, &timeout);
	timeout.tv_sec += 5;

	if (sem_timedwait(&(dev->sem_dev), &timeout)) {
		device_info(dev, "could not aquire cmd semaphore");
		sem_destroy(&(dev->sem_dev));
		sem_init(&(dev->sem_dev), 0, 1);
		sem_wait(&(dev->sem_dev));
	}
	sendto(bl_socket, (char*)buffer, BL_HEAD_LENGTH + enc_len, 0, (struct sockaddr *)&(dev->addr), sizeof(struct sockaddr_in));
}

int bl_decrypt_response(BL_DEVICE *dev, uint8_t *buffer, int received)
{
	uint8_t		aes_ctx[300];
	uint8_t		iv[16];
	int			enc_len;

	if (received <= BL_HEAD_LENGTH)
		return -1;

	enc_len = (received - BL_HEAD_LENGTH + (1 << 4) - 1) & ~((1 << 4) - 1);
	memcpy(iv, bl_master_iv, sizeof(bl_master_iv));
	func_aes_setkey_dec(&aes_ctx, dev->key, 128);
	func_aes_crypt_cbc(&aes_ctx, 0, enc_len, iv, buffer + BL_HEAD_LENGTH, buffer + BL_HEAD_LENGTH);

	if (loglevel >= LOG_DEBUG) {
		char	b[1024];
		int	i, l = 0;

		for (i=0; i < BL_HEAD_LENGTH; i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		b[l++] = '\n';
		for (i=BL_HEAD_LENGTH; i < received; i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		_log(LOG_DEBUG, "data after decrypt: (%d)\n%s", received, b);
	}
	
	return 0;
}

void get_local_ip(int socket, char* iface_name, struct sockaddr_in *addr)
{
	int		i;
	struct ifconf	ifc;
	struct ifreq	*ifr;
	char		buffer[0x400];

	ifc.ifc_len = sizeof(buffer);
	ifc.ifc_buf = (char*)buffer;
	ioctl(socket, SIOCGIFCONF, &ifc);
	ifr = ifc.ifc_req;

	for(i=0; i<(ifc.ifc_len/sizeof(*ifr)); i++)
		if (0 == strcmp(iface_name, ifr[i].ifr_name))
			addr->sin_addr = ((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr;
}

void bl_handle_sent(BL_DEVICE *dev, uint8_t *buffer, int len)
{
	if (loglevel >= LOG_DEBUG) {
		device_info(dev, "send IR/RF completed");
	}
}

void bl_handle_temperature(BL_DEVICE *dev, uint8_t *buffer, int len)
{
	int		temp;

	if (len < 6)
		return;
	
	temp = buffer[4] * 10 + (buffer[5] % 10);
	if (dev->temperature != temp) {
		char	str_temp[32];

		dev->temperature = temp;
		snprintf(str_temp, sizeof(str_temp), "temperature %d.%d", temp / 10, temp % 10);
		device_info(dev, str_temp);	
	}
}

void bl_request_temperature(BL_DEVICE *dev)
{
	uint8_t	buffer[BL_HEAD_LENGTH + 0x10];

	memset(&buffer, 0, sizeof(buffer));
	buffer[BL_OFF_MODE] = BL_MODE_CMD_REQ;
	buffer[BL_HEAD_LENGTH] = BL_CMD_TEMP;
	bl_send_cmd(dev, buffer, BL_HEAD_LENGTH + 0x10);
}

void bl_handle_cmd(BL_DEVICE *dev, uint8_t *buffer, int len)
{
	int			cmd;
	
	if (len < BL_HEAD_LENGTH)
		return;
	
	bl_decrypt_response(dev, buffer, len);
	cmd = buffer[BL_HEAD_LENGTH];

	if (cmd == BL_CMD_TEMP)
		bl_handle_temperature(dev, buffer + BL_HEAD_LENGTH, len - BL_HEAD_LENGTH);
	else if (cmd == BL_CMD_SEND)
		bl_handle_sent(dev, buffer + BL_HEAD_LENGTH, len - BL_HEAD_LENGTH);
}

void bl_handle_auth(BL_DEVICE *dev, uint8_t *buffer, int len)
{
	if (len < BL_HEAD_LENGTH + 0x14)
		return;
	
	bl_decrypt_response(dev, buffer, len);
	memcpy(dev->id,  buffer + BL_HEAD_LENGTH, 4);
	memcpy(dev->key, buffer + BL_HEAD_LENGTH + 4, 16);
	device_info(dev, "authenticated");

	bl_request_temperature(dev);
}

void bl_auth(BL_DEVICE *dev)
{
	uint8_t	buffer[BL_HEAD_LENGTH + 0x50];

	memcpy(dev->key, bl_master_key, sizeof(bl_master_key));

	memset(&buffer, 0, sizeof(buffer));
	buffer[BL_OFF_MODE] = BL_MODE_AUTH_REQ;
	gethostname((char*)(buffer + BL_HEAD_LENGTH + 4), 15);
	buffer[BL_HEAD_LENGTH + 0x1e] = 0x01;
	buffer[BL_HEAD_LENGTH + 0x2d] = 0x01;
	strcpy((char*)buffer + BL_HEAD_LENGTH + 0x30, "xpl");
	bl_send_cmd(dev, buffer, BL_HEAD_LENGTH + 0x50);
}

void bl_handle_discovery(uint8_t *buffer, int len, struct sockaddr_in *addr_from)
{
	BL_DEVICE	*dev = bl_dev + bl_dev_count++;

	memset(dev, 0, sizeof(BL_DEVICE));
	memcpy(dev->mac, buffer + 0x3a, 6);
	memcpy(&dev->addr, addr_from, sizeof(*addr_from));
	dev->devtype = buffer[0x34] | buffer[0x35] << 8;
	get_dev_name(dev->devtype, dev->devtype_name);
	if (len >= 0x4f)
		memcpy(&dev->dev_name, buffer+0x40, 0x0f);

	sem_init(&dev->sem_dev, 0, 1);
	device_info(dev, "discovered");
	
	bl_auth(dev);
}

void bl_discover()
{
	const time_t		now = time(0);
	time_t			time_diff;
	struct tm		tm_local;
	uint8_t			buffer[0x30];
	uint16_t			checksum;
	int				i;
	struct sockaddr_in	addr;

	bl_dev_count = 0;

	tm_local = *localtime(&now);
	time_diff = now - mktime(gmtime(&now));
	time_diff /= 3600;
	tm_local.tm_year += 1900;

	memset(buffer, 0, sizeof(buffer));
	buffer[0x08] = time_diff         & 0xff;
	buffer[0x09] = (time_diff >> 8)  & 0xff;
	buffer[0x0a] = (time_diff >> 16) & 0xff;
	buffer[0x0b] = (time_diff >> 24) & 0xff;
	buffer[0x0c] = tm_local.tm_year & 0xff;
	buffer[0x0d] = tm_local.tm_year >> 8;
	buffer[0x0e] = tm_local.tm_min;
	buffer[0x0f] = tm_local.tm_hour;
	buffer[0x10] = tm_local.tm_year % 100;
	buffer[0x11] = tm_local.tm_wday == 0 ? 7 : tm_local.tm_wday;
	buffer[0x12] = tm_local.tm_mday;
	buffer[0x13] = tm_local.tm_mon + 1;

	memcpy(buffer+0x18, &local_addr.sin_addr.s_addr, 4);
	memcpy(buffer+0x1c, &local_addr.sin_port, 2);
	buffer[BL_OFF_MODE] = BL_MODE_DISCOVERY_REQ;

	for (checksum = 0xbeaf, i = 0; i < 0x30; i++)
		checksum += buffer[i];

	buffer[0x20] = checksum & 0xff;
	buffer[0x21] = checksum >> 8;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addr.sin_port = htons(80);

	if (loglevel >= LOG_DEBUG) {
		char	b[1024];
		int	i, l = 0;

		for (i=0; i < sizeof(buffer); i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		_log(LOG_DEBUG, "Sending discovery request: %s", b);
	}

	sendto(bl_socket, (char*)buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, sizeof(addr));
}

void bl_io_handler(int signal)
{
	uint8_t			buffer[0x200];
	socklen_t			len;
	struct sockaddr_in	addr_from;
	int				received;

	len = sizeof(addr_from);
	received = recvfrom(bl_socket, (char*)buffer, sizeof(buffer), 0, (struct sockaddr *)&addr_from, &len);

	if (received == 0)
		return;
	
	if (loglevel >= LOG_DEBUG) {
		char	b[4096];
		int	i, l = 0;

		for (i=0; i < 0x38; i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		b[l++] = '\n';
		for (i=0x38; i < received; i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		_log(LOG_DEBUG, "received: (%d) %s", received, b);
	}

	if (received < BL_OFF_MODE)
		return;

	if (buffer[BL_OFF_MODE]  == BL_MODE_DISCOVERY_RESP) {
		bl_handle_discovery(buffer, received, &addr_from);
		return;
	}

	BL_DEVICE	*dev = bl_find_device_by_addr(&addr_from);

	if (!dev)
		return;
	
	sem_post(&dev->sem_dev);

	if (buffer[BL_OFF_MODE]  == BL_MODE_AUTH_RESP) {
		bl_handle_auth(dev, buffer, received);
		return;
	}

	if (buffer[BL_OFF_MODE]  == BL_MODE_CMD_RESP)
		bl_handle_cmd(dev, buffer, received);
}

size_t bl_write_frame(void *unused, uint8_t *buffer, size_t len, uint16_t time_ms)
{
	uint32_t	units;

	units   = time_ms;
	units  *= 269;
	units  += 1 << 12;	// 4096 for rounding
	units >>= 13;		// 8192

	if (units < 0xff) {
		if (!len)
			return 0;

		*buffer = units;
		return 1;
	}

	if (len < 3)
		return 0;

	buffer[0] = 0;
	buffer[1] = (units >> 8) & 0xff;
	buffer[2] = units & 0xff;
	return 3;
}

char* bl_get_var(void *ctx, char *name)
{
	xPL_MessagePtr msg = ctx;

	return xPL_getMessageNamedValue(msg, name);
}

void bl_send_code(BL_DEVICE *dev, CODEDEF *code, xPL_MessagePtr msg)
{
	uint8_t		buffer[0x200];
	size_t		pos;
	int		len;
	uint8_t		cmd[32];
	int		repeat;
	char		*repeat_str;

	if (!ir_rf_cmd_from_vars(code, (callback_get_var)xPL_getMessageNamedValue, msg, cmd, sizeof(cmd)))
		return;

	memset(&buffer, 0, sizeof(buffer));
	buffer[BL_OFF_MODE] = BL_MODE_CMD_REQ;

	pos = BL_HEAD_LENGTH;
	buffer[pos] = BL_CMD_SEND;
	pos += 4;
	if (code->code_type == CODE_TYPE_IR) {
		buffer[pos++] = 0x26;	//0b0010 0110
		repeat = 0;
	} else {
		buffer[pos++] = 0xb2;	//0b1011 0010
		repeat = 5;
	}

	if (code->repeat)
		repeat = code->repeat;

	repeat_str = xPL_getMessageNamedValue(msg, "repeat");
	if (repeat_str)
		repeat = strtol(repeat_str, 0, 0);

	buffer[pos++] = repeat;

	pos += 2;
	pos += ir_rf_generate_pulses(code, cmd, sizeof(cmd), buffer + pos, sizeof(buffer) - pos, bl_write_frame, 0);

	if (pos & 1)
		buffer[pos++] = 0;

	if (code->code_type == CODE_TYPE_IR) {
		buffer[pos++] = 0x0d;
		buffer[pos++] = 0x05;
	}

	len = pos - BL_HEAD_LENGTH - 8;
	buffer[BL_HEAD_LENGTH + 6] = len & 0xff;
	buffer[BL_HEAD_LENGTH + 7] = len >> 8;

	if (loglevel >= LOG_DEBUG) {
		char	b[1024];
		int	i, l = 0;

		for (i=0x38; i < pos; i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		_log(LOG_DEBUG, "IR/RF data: (%d)\n%s", pos, b);
	}

	bl_send_cmd(dev, buffer, pos);


	if (loglevel >= LOG_NOTICE) {
		char			info[256];
		size_t			info_pos;
		xPL_NameValueList	*body;
		xPL_NameValuePair	**nv;
		int			i;

		info_pos = snprintf(info, sizeof(info), "%s-%s.%s {",
			xPL_getSourceVendor(msg), xPL_getSourceDeviceID(msg), xPL_getSourceInstanceID(msg));

		body = xPL_getMessageBody(msg);
		for (i = 0, nv = body->namedValues; i < body->namedValueCount; i++)
			info_pos += snprintf(info + info_pos, sizeof(info) - info_pos, " %s=%s", nv[i]->itemName, nv[i]->itemValue);

		info_pos += snprintf(info + info_pos, sizeof(info) - info_pos, " } => code %s, (repeat=%d) ",
			code->name, repeat);
		for (i = 0; i < (code->bits + 7) >> 3; i++)
			info_pos += snprintf(info + info_pos, sizeof(info) - info_pos, "%02x", cmd[i]);

		device_info(dev, info);
	}
}

void xpl_send_code(BL_DEVICE *dev, xPL_MessagePtr msg)
{
	char	*code_name;
	CODEDEF	*code;
	int	found = 0;

	code_name = xPL_getMessageNamedValue(msg, "code");
	if (!code_name) {
		_log(LOG_ERR, "Code not specified");
		return;
	}

	for (code = codes.entries; code < codes.entries + codes.count; code++) {
		if (strcmp(code_name, code->name))
			continue;

		found = 1;
		bl_send_code(dev, code, msg);
	}

	if (!found)
		_log(LOG_ERR, "Unknown code %s", code_name);
}

void bl_msg_handler(xPL_Message *msg, xPL_ObjectPtr data)
{
	BL_DEVICE	*d, *dev;
	char		*vendor, *device, *instance, *msg_class, *msg_type;
	uint16_t	device_id;
	size_t		len;
	int		tries;

	char	*my_vendor_id = data;
	
	vendor = xPL_getTargetVendor(msg);
	if (vendor == 0 || strcmp(vendor, my_vendor_id))
		return;

	device = xPL_getTargetDeviceID(msg);
	if (device == 0)
		return;

	instance = xPL_getTargetInstanceID(msg);
	if (instance == 0)
		return;

	msg_class = xPL_getSchemaClass(msg);
	if (msg_class == 0 || strcmp(msg_class, "control"))
		return;

	msg_type = xPL_getSchemaType(msg);
	if (msg_type == 0 || strcmp(msg_type, "basic"))
		return;

	len = strlen(device);
	device_id = strtol(device, 0, 16);

	for (dev = 0, tries = 0; !dev && tries < 2; tries++) {
		if (tries)
			bl_discover();

		for (d = bl_dev; !dev && d < bl_dev + bl_dev_count; d++) {
			if (device_id ?
			     d->devtype != device_id :
			     strncasecmp(device, d->devtype_name, len))
				continue;

			if (strcmp("default", instance)) {
				char	mac[16];

				sprintf(mac, "%02x%02x%02x%02x%02x%02x",
					d->mac[5], d->mac[4], d->mac[3],
					d->mac[2], d->mac[1], d->mac[0]);
				if (strcmp(mac, instance))
					continue;
			}

			dev = d;
		}
	}

	if (!dev) {
		_log(LOG_ERR, "No device %s found", device);
		return;
	}

	xpl_send_code(dev, msg);
}

int bl_startup()
{
	int		rc;
	socklen_t	len;
	uint32_t	ctrue = 1;
	
	cryptolib = dlopen("libmbedcrypto.so", RTLD_NOW);
	if (!cryptolib)
		cryptolib = dlopen("libpolarssl.so", RTLD_NOW);

	if (!cryptolib) {
		_log(LOG_NOTICE, "Could not open SSL library -> Broadlink xPL gateway not started");
		return -1;
	} else {
		func_aes_crypt_cbc  = dlsym(cryptolib, "mbedtls_aes_crypt_cbc");
		func_aes_setkey_enc = dlsym(cryptolib, "mbedtls_aes_setkey_enc");
		func_aes_setkey_dec = dlsym(cryptolib, "mbedtls_aes_setkey_dec");

		if (!func_aes_crypt_cbc)
			func_aes_crypt_cbc  = dlsym(cryptolib, "aes_crypt_cbc");
		if (!func_aes_setkey_enc)
			func_aes_setkey_enc = dlsym(cryptolib, "aes_setkey_enc");
		if (!func_aes_setkey_dec)
			func_aes_setkey_dec = dlsym(cryptolib, "aes_setkey_dec");

		if (!func_aes_crypt_cbc || !func_aes_setkey_enc || !func_aes_setkey_dec) {
			dlclose(cryptolib);
			cryptolib = 0;
			_log(LOG_NOTICE, "AES functions not found in SSL library -> Broadlink xPL gateway not started");
			return -1;
		}
	}

	bl_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	get_local_ip(bl_socket, iface, &local_addr);

	rc = bind(bl_socket, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (rc != 0) {
		_log(LOG_ERR, "bind failed");
		return -1;
	}
	len = sizeof(local_addr);
	getsockname(bl_socket, (struct sockaddr *)&local_addr, &len);

	setsockopt(bl_socket, SOL_SOCKET, SO_BROADCAST, (char*)&ctrue, sizeof(ctrue));
	setsockopt(bl_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&ctrue, sizeof(ctrue));

	fcntl(bl_socket, F_SETOWN, getpid());
	fcntl(bl_socket, F_SETFL, FASYNC);

	bl_discover();
	return 0;
}

void bl_shutdown()
{
	if (cryptolib)
		dlclose(cryptolib);
}