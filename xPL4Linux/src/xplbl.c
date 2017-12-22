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

#include "xPL.h"

#include "args.h"
#include "code_ir_rf.h"
#include "log.h"

typedef struct {
	uint16_t	id;
	const char	*name;
} DEV_NAME;

typedef struct {
	struct sockaddr_in	addr;
	uint8_t			mac[6];
	uint16_t		devtype;
	char			devtype_name[16];
	uint16_t		count;
	uint8_t			key[16];
	uint8_t			id[4];
	int			temperature;
} BL_DEVICE;

#define BL_HEAD_LENGTH		0x38
#define BL_OFF_MODE		0x26
#define BL_MODE_AUTH		0x65
#define BL_MODE_CMD		0x6a
#define BL_CMD_TEMP		0x01
#define BL_CMD_SEND		0x02
#define BL_CMD_LEARN		0x03
#define BL_CMD_LEARN_DATA	0x04

#define DISCOVERY_TIMEOUT	2

static const DEV_NAME dev_names[] = {
	{ 0x272a, "RM2_Pro_Plus" }
};

static const int dev_names_count = sizeof(dev_names) / sizeof(dev_names[0]);

uint8_t	bl_master_key[] = { 0x09, 0x76, 0x28, 0x34, 0x3f, 0xe9, 0x9e, 0x23, 0x76, 0x5c, 0x15, 0x13, 0xac, 0xcf, 0x8b, 0x02 };
uint8_t	bl_master_iv[]  = { 0x56, 0x2e, 0x17, 0x99, 0x6d, 0x09, 0x3d, 0x28, 0xdd, 0xb3, 0xba, 0x69, 0x5a, 0x2e, 0x6f, 0x58 };

MAPS maps_bl = { 0, 0, 0 };
CODEDEFS codes_bl = { &maps_bl, 0, 0, 0 };

BL_DEVICE	bl_dev[8];
int		bl_dev_count = 0;
char		iface[IFNAMSIZ] = "br-lan";
xPL_ServicePtr	xpl_service = 0;

#define VERSION "1.0"

static const char vendor_id[] = "broadlink";

typedef int (*ptr_aes_crypt_cbc)(void*, int, size_t, void*, void*, void*);
typedef int (*ptr_aes_setkey)(void*, void*, unsigned int);

void *cryptolib = 0;
ptr_aes_crypt_cbc func_aes_crypt_cbc = 0;
ptr_aes_setkey func_aes_setkey_enc = 0;
ptr_aes_setkey func_aes_setkey_dec = 0;


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
	_log(LOG_NOTICE, "Device [%s], %d.%d.%d.%d (%02x:%02x:%02x:%02x:%02x:%02x) %s",
		dev->devtype_name,
		dev->addr.sin_addr.s_addr >> 24,
		(dev->addr.sin_addr.s_addr >> 16) &0xff,
		(dev->addr.sin_addr.s_addr >> 8) &0xff,
		dev->addr.sin_addr.s_addr &0xff,
		dev->mac[5], dev->mac[4], dev->mac[3],
		dev->mac[2], dev->mac[1], dev->mac[0],
		info);
}

bool set_iface(char** args, void *unused)
{
	strncpy(iface, *args, sizeof(iface));
	return true;
}

int bl_send_command(BL_DEVICE *bl_dev, uint8_t *buffer, int in_len, int out_len, int timeout)
{
	uint8_t		aes_ctx[300];;
	uint8_t		iv[16];
	int		s;
	uint16_t	checksum;
	int		i;
	int		received, enc_len;
	struct timeval tv;

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

	bl_dev->count++;
	buffer[0x28] = bl_dev->count & 0xff;
	buffer[0x29] = bl_dev->count >> 8;
	memcpy(buffer + 0x2a, bl_dev->mac, 6);
	memcpy(buffer + 0x30, bl_dev->id, 4);

	for (checksum = 0xbeaf, i = BL_HEAD_LENGTH; i < in_len; i++)
		checksum += buffer[i];

	buffer[0x34] = checksum & 0xff;
	buffer[0x35] = checksum >> 8;

	enc_len = (in_len - 0x38 + (1 << 4) - 1) & ~((1 << 4) - 1);
	memcpy(iv, bl_master_iv, sizeof(bl_master_iv));
	func_aes_setkey_enc(&aes_ctx, bl_dev->key, 128);
	func_aes_crypt_cbc(&aes_ctx, 1, enc_len, iv, buffer + BL_HEAD_LENGTH, buffer + BL_HEAD_LENGTH);

	for (checksum = 0xbeaf, i = 0; i < BL_HEAD_LENGTH + enc_len; i++)
		checksum += buffer[i];

	buffer[0x20] = checksum & 0xff;
	buffer[0x21] = checksum >> 8;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

	sendto(s, (char*)buffer, BL_HEAD_LENGTH + enc_len, 0, (struct sockaddr *)&(bl_dev->addr), sizeof(struct sockaddr_in));
	received = recv(s, (char*)buffer, out_len, 0);

	if (loglevel >= LOG_DEBUG) {
		char	b[1024];
		int	i, l = 0;

		for (i=0; i < 0x38; i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		b[l++] = '\n';
		for (i=0x38; i < received; i++)
			l += sprintf(b + l, "%02x:", buffer[i]);
		_log(LOG_DEBUG, "received: (%d) %s", received, b);
	}

	if (received > BL_HEAD_LENGTH) {
		enc_len = (received - 0x38 + (1 << 4) - 1) & ~((1 << 4) - 1);
		memcpy(iv, bl_master_iv, sizeof(bl_master_iv));
		func_aes_setkey_dec(&aes_ctx, bl_dev->key, 128);
		func_aes_crypt_cbc(&aes_ctx, 0, enc_len, iv, buffer + BL_HEAD_LENGTH, buffer + BL_HEAD_LENGTH);

		if (loglevel >= LOG_DEBUG) {
			char	b[1024];
			int	i, l = 0;

			for (i=0; i < 0x38; i++)
				l += sprintf(b + l, "%02x:", buffer[i]);
			b[l++] = '\n';
			for (i=0x38; i < received; i++)
				l += sprintf(b + l, "%02x:", buffer[i]);
			_log(LOG_DEBUG, "received after decrypt: (%d)\n%s", received, b);
		}
	}

	close(s);
	return received;
}

int bl_get_temperature(BL_DEVICE *bl_dev)
{
	uint8_t		buffer[0x200];
	int		len, temperature;

	memset(&buffer, 0, sizeof(buffer));
	buffer[BL_OFF_MODE] = BL_MODE_CMD;
	buffer[BL_HEAD_LENGTH] = BL_CMD_TEMP;
	len = bl_send_command(bl_dev, buffer, BL_HEAD_LENGTH + 0x10, sizeof(buffer), 1000);

	if (len < BL_HEAD_LENGTH + 6)
		return 0;

	temperature = buffer[BL_HEAD_LENGTH + 4] * 10 + (buffer[BL_HEAD_LENGTH + 5] % 10);
	_log(LOG_INFO,"Temperature: %d.%d", temperature / 10, temperature % 10);
	return temperature;
}

int bl_auth(BL_DEVICE *bl_dev)
{
	uint8_t		buffer[0x200];
	int		len;

	memcpy(bl_dev->key, bl_master_key, sizeof(bl_master_key));

	memset(&buffer, 0, sizeof(buffer));
	buffer[BL_OFF_MODE] = BL_MODE_AUTH;
	gethostname((char*)(buffer + BL_HEAD_LENGTH + 4), 15);
	buffer[BL_HEAD_LENGTH + 0x1e] = 0x01;
	buffer[BL_HEAD_LENGTH + 0x2d] = 0x01;
	strcpy((char*)buffer + BL_HEAD_LENGTH + 0x30, "xpl");
	len = bl_send_command(bl_dev, buffer, BL_HEAD_LENGTH + 0x50, sizeof(buffer), 5000);

	if (len < BL_HEAD_LENGTH + 0x14)
		return -1;

	memcpy(bl_dev->id,  buffer + BL_HEAD_LENGTH, 4);
	memcpy(bl_dev->key, buffer + BL_HEAD_LENGTH + 4, 16);
	device_info(bl_dev, "authenticated");
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

void bl_discover()
{
	int		s;
	int		rc;
	uint32_t	ctrue = 1;
	struct sockaddr_in	local_addr, addr, addr_from;
	socklen_t	len;
	int received;

	uint8_t		buffer[0x200];
	struct tm	tm_local;
	const time_t	now = time(0);
	time_t		time_diff, time_start, time_remain;
	uint16_t	checksum;
	int		i;

	bl_dev_count = 0;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	memset(&local_addr, 0, sizeof(local_addr));
	local_addr.sin_family = AF_INET;
	get_local_ip(s, iface, &local_addr);

	rc = bind(s, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if (rc != 0) {
		_log(LOG_ERR, "bind failed");
		return;
	}
	len = sizeof(local_addr);
	getsockname(s, (struct sockaddr *)&local_addr, &len);

	setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char*)&ctrue, sizeof(ctrue));
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&ctrue, sizeof(ctrue));

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
	buffer[0x26] = 6;

	for (checksum = 0xbeaf, i = 0; i < 0x30; i++)
		checksum += buffer[i];

	buffer[0x20] = checksum & 0xff;
	buffer[0x21] = checksum >> 8;

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addr.sin_port = htons(80);

	bl_dev_count = 0;
	rc = sendto(s, (char*)buffer, 0x30, 0, (struct sockaddr *)&addr, sizeof(addr));

	time_start = time(0);
	time_remain = DISCOVERY_TIMEOUT;
	while (time_remain > 0 && bl_dev_count < sizeof(bl_dev)/sizeof(bl_dev[0])) {
		struct timeval tv;

		tv.tv_sec = time_remain;
		tv.tv_usec = 0;
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

		len = sizeof(addr_from);
		received = recvfrom(s, (char*)buffer, sizeof(buffer), 0, (struct sockaddr *)&addr_from, &len);

		if (received > 0) {
			BL_DEVICE	*dev = bl_dev + bl_dev_count;

			memset(dev, 0, sizeof(BL_DEVICE));
			memcpy(dev->mac, buffer + 0x3a, 6);
			memcpy(&dev->addr, &addr_from, sizeof(addr_from));
			dev->devtype = buffer[0x34] | buffer[0x35] << 8;
			get_dev_name(dev->devtype, dev->devtype_name);
			bl_dev_count++;
			device_info(dev, "discovered");
		}

		time_remain = time_start + DISCOVERY_TIMEOUT - time(0);
	}

	for (i = 0; i < bl_dev_count; i++)
		bl_auth(bl_dev + i);
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
	buffer[BL_OFF_MODE] = BL_MODE_CMD;

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

	len = bl_send_command(dev, buffer, pos, sizeof(buffer), 500);
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

	for (code = codes_bl.entries; code < codes_bl.entries + codes_bl.count; code++) {
		if (strcmp(code_name, code->name))
			continue;

		found = 1;
		bl_send_code(dev, code, msg);
	}

	if (!found)
		_log(LOG_ERR, "Unknown code %s", code_name);
}

void xpl_msg_handler(xPL_MessagePtr msg, xPL_ObjectPtr data)
{
	BL_DEVICE	*d, *dev;
	char		*vendor, *device, *instance, *msg_class, *msg_type;
	uint16_t	device_id;
	size_t		len;
	int		tries;

	vendor = xPL_getTargetVendor(msg);
	if (vendor == 0 || strcmp(vendor_id, vendor))
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

void xpl_broadlink_shutdownHandler(int onSignal) {
	if (xpl_service) {
		xPL_setServiceEnabled(xpl_service, FALSE);
		xPL_releaseService(xpl_service);
	}

	xPL_shutdown();
	dlclose(cryptolib);
	_log(LOG_INFO, "Shutdown");
	exit(0);
}

int xpl_broadlink_main(int argc, String argv[]) {
	static const PARAM	params[] = {
		{ "n", "net", "Set network interface", 1, set_iface },
		{ "m", "map", "Value map", 1, add_map, &maps_bl },
		{ "c", "code", "Code definition", 1, add_code, &codes_bl },
	};

	cryptolib = dlopen("libmbedcrypto.so", RTLD_NOW);
	if (!cryptolib)
		cryptolib = dlopen("libpolarssl.so", RTLD_NOW);

	if (!cryptolib) {
		_log(LOG_EMERG, "Could not open SSL library");
		return -1;
	}

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
		_log(LOG_EMERG, "AES functions not found in SSL library");
		return -1;
	}

	if (!xPL_parseCommonArgs(&argc, argv, FALSE)) {
		_log(LOG_ERR, "Unable to start xPL");
		return -1;
	}

	if (!parse_args(argc, argv, params, sizeof(params)/sizeof(params[0])))
		return -2;

	bl_discover();

//	for (i = 0; i< bl_dev_count; i++) {
//		bl_dev[i].temperature = bl_get_temperature(bl_dev + i);
//	}

	dump_maps(&maps_bl);
	dump_codes(&codes_bl);

	/* Start xPL up */
	if (!xPL_initialize(xPL_getParsedConnectionType())) {
		_log(LOG_ERR, "Unable to start xPL");
		return -3;
	}

	_log(LOG_INFO, "Startup");
	xpl_service =  xPL_createService((char*)vendor_id, "default", "default");
	xPL_setServiceVersion(xpl_service, VERSION);
	xPL_setServiceEnabled(xpl_service, TRUE);

	/* And a listener for all xPL messages */
	xPL_addMessageListener(xpl_msg_handler, NULL);

	/* Install signal traps for proper shutdown */
	signal(SIGTERM, xpl_broadlink_shutdownHandler);
	signal(SIGINT,  xpl_broadlink_shutdownHandler);

	/** Main Loop  **/
	for (;;) {
	/* Let XPL run for a while, returning after it hasn't seen any */
	/* activity in 100ms or so                                     */
		xPL_processMessages(100);
	}
}

