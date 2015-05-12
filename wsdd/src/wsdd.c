/*
 * wsdd.c - WS Discovery and LLMNR daemon
 *
 * Copyright (c) 2013 Tobias Waldvogel.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Anounces a device to Windows via WSD provides name resoulution via LLMNR
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>

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

#include <time.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include <signal.h>
 
#define LLMNR_PORT 5355
#define LLMNR_MCAST_ADDR ("224.0.0.252")
#define WSD_PORT 3702
#define WSD_HTTP_PORT  WSD_PORT
#define WSD_MCAST_ADDR ("239.255.255.250")
#define DNS_TYPE_A 1

#define MAX_CLIENTS	29
#define WSD_HTTP_SOCK	0
#define WSD_UDP_SOCK	1
#define LLMNR_UDP_SOCK	2

static const char wsd_act_hello[] = "Hello";
static const char wsd_act_bye[] = "Bye";
static const char wsd_act_resolve[] = "Resolve";
static const char wsd_act_resolve_matches[] = "ResolveMatches";
static const char wsd_act_probe[] = "Probe";
static const char wsd_act_probe_matches[] = "ProbeMatches";
static const char wsd_act_get[] = "Get";
static const char wsd_act_get_response[] = "GetResponse";
static const char wsd_device[] = "wsdp:Device pub:Computer";

static const char wsd_discovery[] = "http://schemas.xmlsoap.org/ws/2005/04/discovery/";
static const char wsd_transfer[] = "http://schemas.xmlsoap.org/ws/2004/09/transfer/";

static const char wsd_to_anon[] = "http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous";
static const char wsd_to_discovery[] = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";

static const char wsd_addr[]   = ":Address>";
static const char wsd_msgid[]  = ":MessageID>";
static const char wsd_body[]   = ":Body>";
static const char wsd_types[]  = ":Types>";

/* global/static variables */
int	loglevel = LOG_ERR;
int	asdaemon = 1;
int	usesyslog = 1;
int	terminate = 0;
char	endpoint[48];
char 	sequence[48];
int	instance_id;
int	msg_no = 1;
char	in[8192], out[8192];

/* Variable for computer device */
char	cd_name[128];
char	cd_workgroup[128] = "WORKGROUP";
char	cd_friendly_name[128] = "wsdd enabled device";
char	cd_url[256] = "http://specs.xmlsoap.org/ws/2006/02/devprof";
char	cd_manufacturer[128] = "wsdd";
char	cd_model[128] = "wsdd";
char	cd_serial[32] = "1";
char	cd_firmware[16] = "1.0";

void wsdd_log(int priority, const char* format, ...) {
	va_list va;
	char printbuf[1024];

	if (priority > loglevel)
		return;

	va_start(va, format);
	vsnprintf(printbuf, sizeof(printbuf), format, va);

	if (usesyslog)
		syslog(priority, "%s", printbuf);
	else
		fprintf(stderr, "%s\n", printbuf);
}

void daemonize(void)
{
	FILE *fp;
	int maxfd;
	int i;

	/* fork #1: exit parent process and continue in the background */
	if ((i = fork()) < 0) {
		perror("couldn't fork");
		exit(2);
	} else if (i > 0) {
		_exit(0);
	}

	/* fork #2: detach from terminal and fork again so we can never regain
	* access to the terminal */
	setsid();
	if ((i = fork()) < 0) {
		perror("couldn't fork #2");
		exit(2);
	} else if (i > 0) {
		_exit(0);
	}

	/* write pid */
	if ((fp=fopen("/var/run/wsdd.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* change to root directory and close file descriptors */
	if (chdir("/"))
		wsdd_log(LOG_INFO, "chdir / failed");
	maxfd = getdtablesize();
	for (i = 0; i < maxfd; i++)
		close(i);

	/* use /dev/null for stdin, stdout and stderr */
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_WRONLY);
	open("/dev/null", O_WRONLY);
}

/* Find xml tag value */
char* get_tag_value(char *xml, const char *tag, int taglen, int *len)
{
	char *val, *end;

	val = strstr(xml, tag);
	if (!val)
		return NULL;

	val += taglen;

	end = strstr(val, "<");
	if (!end)
		return NULL;

	*len = end - val;
	return val;
}

int gen_soap_header(char *buffer, int *len, const char *to, 
				const char* action_pre, const char *action, const char *relates, int http)
{
	static const char wsd_soap_header_start[] =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<soap:Envelope "
			"xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" "
			"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
			"xmlns:wsd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
			"xmlns:wsx=\"http://schemas.xmlsoap.org/ws/2004/09/mex\" "
			"xmlns:wsdp=\"http://schemas.xmlsoap.org/ws/2006/02/devprof\" "
			"xmlns:un0=\"http://schemas.microsoft.com/windows/pnpx/2005/10\" "
			"xmlns:pub=\"http://schemas.microsoft.com/windows/pub/2005/07\">"
		"<soap:Header>"
			"<wsa:To>%s</wsa:To>"
			"<wsa:Action>%s%s</wsa:Action>"
			"<wsa:MessageID>urn:uuid:%s</wsa:MessageID>";
	static const char wsd_soap_header_relates[] =
			"<wsa:RelatesTo>%s</wsa:RelatesTo>";
	static const char wsd_soap_header_seq[] =
			"<wsd:AppSequence InstanceId=\"%d\" SequenceId=\"%s\" MessageNumber=\"%d\" />";
	static const char wsd_soap_header_end[] =
		"</soap:Header>";
			
	char	msg_id[37];
	uuid_t	uu;
	int		ret_len;

	uuid_generate_time(uu);
	uuid_unparse(uu, msg_id);

	ret_len = snprintf(buffer, *len, wsd_soap_header_start, to, action_pre, action, msg_id);
	if (ret_len >= *len)
		return -1;

	if (relates) {
		ret_len += snprintf(buffer+ret_len, (*len)-ret_len, wsd_soap_header_relates, relates);
		if (ret_len >= *len)
			return -1;
	}
	
	if (!http) {
		ret_len += snprintf(buffer+ret_len, (*len)-ret_len, wsd_soap_header_seq,
				instance_id, sequence, msg_no++);
		if (ret_len >= *len)
			return -1;
	}

	ret_len += snprintf(buffer+ret_len, (*len)-ret_len, wsd_soap_header_end);
	*len = ret_len;
	return 0;
}

int action_hello(char* out, int *out_len, const char* service, int http)
{
	static const char wsd_hello[] =
		"<soap:Body>"
		  "<wsd:Hello>"
			"<wsa:EndpointReference>"
			"<wsa:Address>%s</wsa:Address>"
			"</wsa:EndpointReference>"
			"<wsd:Types>%s</wsd:Types>"
			"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
		  "</wsd:Hello>"
		"</soap:Body>"
	"</soap:Envelope>";

	int		ret_len;

	ret_len = *out_len;
	if (gen_soap_header(out, &ret_len, wsd_to_discovery, wsd_discovery, wsd_act_hello, 0, http) < 0)
		return -1;
		
	ret_len += snprintf(out+ret_len, (*out_len)-ret_len, wsd_hello, endpoint, service);
	*out_len = ret_len;
	return 0;
}

int action_bye(char* out, int *out_len, const char* service, int http)
{
	static const char wsd_bye[] =
		"<soap:Body>"
		  "<wsd:Bye>"
			"<wsa:EndpointReference>"
			"<wsa:Address>%s</wsa:Address>"
			"</wsa:EndpointReference>"
			"<wsd:Types>%s</wsd:Types>"
			"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
		  "</wsd:Bye>"
		"</soap:Body>"
	"</soap:Envelope>";

	int		ret_len;

	ret_len = *out_len;
	if (gen_soap_header(out, &ret_len, wsd_to_discovery, wsd_discovery, wsd_act_bye, 0, http) < 0)
		return -1;
		
	ret_len += snprintf(out+ret_len, (*out_len)-ret_len, wsd_bye, endpoint, service);
	*out_len = ret_len;
	return 0;
}

int action_resolve(char *recv_ip, char *src_msgid, char *body, char* out, int *out_len, int http)
{
	static const char wsd_resolve_match[] =
		"<soap:Body>"
			"<wsd:ResolveMatches>"
				"<wsd:ResolveMatch>"
					"<wsa:EndpointReference>"
						"<wsa:Address>%s</wsa:Address>"
					"</wsa:EndpointReference>"
					"<wsd:Types>%s</wsd:Types>"
					"<wsd:XAddrs>http://%s:%d/wsd/</wsd:XAddrs>"
					"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
				"</wsd:ResolveMatch>"
			"</wsd:ResolveMatches>"
		"</soap:Body>"
	"</soap:Envelope>";

	char	*endp;
	int	endp_len, ret_len;
	
	endp = get_tag_value(body, wsd_addr, sizeof(wsd_addr)-1, &endp_len);
	if (!endp) {
		wsdd_log(LOG_INFO, "Endpoint not found");
		return -1;
	}			
	endp[endp_len] = 0;
	
	if (strcasecmp(endp, endpoint))
		return -1;

	ret_len = *out_len;
	if (gen_soap_header(out, &ret_len, wsd_to_anon, wsd_discovery, wsd_act_resolve_matches, src_msgid, http) < 0)
		return -1;
		
	ret_len += snprintf(out+ret_len, (*out_len)-ret_len, wsd_resolve_match, endpoint, wsd_device, recv_ip, WSD_HTTP_PORT);
	*out_len = ret_len;
	return 0;
}

int action_probe(char *recv_ip, char *src_msgid, char *body, char* out, int *out_len, int http)
{
	static const char wsd_probe_match[] =
		"<soap:Body>"
			"<wsd:ProbeMatches>"
				"<wsd:ProbeMatch>"
					"<wsa:EndpointReference>"
						"<wsa:Address>%s</wsa:Address>"
					"</wsa:EndpointReference>"
					"<wsd:Types>%s</wsd:Types>"
					"<wsd:XAddrs>http://%s:%d/wsd/</wsd:XAddrs>"
					"<wsd:MetadataVersion>2</wsd:MetadataVersion>"
				"</wsd:ProbeMatch>"
			"</wsd:ProbeMatches>"
		"</soap:Body>"
	"</soap:Envelope>";

	char	*types;
	int		types_len, ret_len;
	
	types = get_tag_value(body, wsd_types, sizeof(wsd_types)-1, &types_len);
	if (!types)
		return -1;

	types[types_len] = 0;
	
	if (strncmp(types, wsd_device, types_len))
		return -1;

	ret_len = *out_len;
	if (gen_soap_header(out, &ret_len, wsd_to_anon, wsd_discovery, wsd_act_probe_matches, src_msgid, http) < 0)
		return -1;
		
	ret_len += snprintf(out+ret_len, (*out_len)-ret_len, wsd_probe_match, endpoint, wsd_device, recv_ip, WSD_HTTP_PORT);
	*out_len = ret_len;
	return 0;
}

int action_get(char *src_msgid, char *out, int *out_len, int http)
{
	static const char wsd_getresponse[] =
	   "<soap:Body>"
		  "<wsx:Metadata>"
			 "<wsx:MetadataSection Dialect=\"http://schemas.xmlsoap.org/ws/2006/02/devprof/ThisDevice\">"
				"<wsdp:ThisDevice>"
				   "<wsdp:FriendlyName>%s</wsdp:FriendlyName>"
				   "<wsdp:FirmwareVersion>%s</wsdp:FirmwareVersion>"
				   "<wsdp:SerialNumber>%s</wsdp:SerialNumber>"
				"</wsdp:ThisDevice>"
			 "</wsx:MetadataSection>"
			 "<wsx:MetadataSection Dialect=\"http://schemas.xmlsoap.org/ws/2006/02/devprof/ThisModel\">"
				"<wsdp:ThisModel>"
				   "<wsdp:Manufacturer>%s</wsdp:Manufacturer>"
				   "<wsdp:ManufacturerUrl>%s</wsdp:ManufacturerUrl>"
				   "<wsdp:ModelName>%s</wsdp:ModelName>"
				   "<wsdp:ModelNumber>1</wsdp:ModelNumber>"
				   "<wsdp:ModelUrl>%s</wsdp:ModelUrl>"
				   "<wsdp:PresentationUrl>%s</wsdp:PresentationUrl>"
				   "<un0:DeviceCategory>Computers</un0:DeviceCategory>"
				"</wsdp:ThisModel>"
			 "</wsx:MetadataSection>"
			 "<wsx:MetadataSection Dialect=\"http://schemas.xmlsoap.org/ws/2006/02/devprof/Relationship\">"
				"<wsdp:Relationship Type=\"http://schemas.xmlsoap.org/ws/2006/02/devprof/host\">"
				   "<wsdp:Host>"
					  "<wsa:EndpointReference>"
						 "<wsa:Address>%s</wsa:Address>"
					  "</wsa:EndpointReference>"
					  "<wsdp:Types>pub:Computer</wsdp:Types>"
					  "<wsdp:ServiceId>%s</wsdp:ServiceId>"
					  "<pub:Computer>%s/Workgroup:%s</pub:Computer>"
				   "</wsdp:Host>"
				"</wsdp:Relationship>"
			 "</wsx:MetadataSection>"
		  "</wsx:Metadata>"
		"</soap:Body>"
	  "</soap:Envelope>";

	int		ret_len;

	ret_len = *out_len;
	if (gen_soap_header(out, &ret_len, wsd_to_anon, wsd_transfer, wsd_act_get_response, src_msgid, http) < 0)
		return -1;
		
	ret_len += snprintf(out+ret_len, (*out_len)-ret_len, wsd_getresponse,
					cd_friendly_name, cd_firmware, cd_serial, cd_manufacturer, cd_url,
					cd_model, cd_url, cd_url, endpoint, endpoint, cd_name, cd_workgroup);
	*out_len = ret_len;
	return 0;
}

int handle_request(char *recv_ip, char *in, int in_len, char *out, int *out_len, int http)
{
	char	*action, *msgid, *body;
	int	action_len, msgid_len;

	action = get_tag_value(in, wsd_discovery, sizeof(wsd_discovery)-1, &action_len);
	if (!action)
		action = get_tag_value(in, wsd_transfer, sizeof(wsd_transfer)-1, &action_len);

	if (!action) {
		if (http)
			return action_get(0, out, out_len, http);
			
		return -1;
	}

	msgid = get_tag_value(in, wsd_msgid, sizeof(wsd_msgid)-1, &msgid_len);
	if (!msgid)
		return -1;

	body = strstr(in, wsd_body);
	if (!msgid)
		return -1;

	action[action_len] = 0;
	msgid[msgid_len] = 0;
	
	wsdd_log(LOG_INFO, "Action %s", action);
	if (strcmp(action, wsd_act_probe) == 0)
		return action_probe(recv_ip, msgid, body, out, out_len, http);
	if (strcmp(action, wsd_act_resolve) == 0)
		return action_resolve(recv_ip, msgid, body, out, out_len, http);
	if (strcmp(action, wsd_act_get) == 0)
		return action_get(msgid, out, out_len, http);
	return -1;
}

void wsdd_http_request(int client)
{
	static const char http_response[] =
	"HTTP/1.1 200\r\n" 
	"Content-Type: %s\r\n"
	"Server: wsdd\r\n"
	"Date: %s\r\n"
	"Connection: close\r\n"
	"Content-Length: %d\r\n\r\n";

	static const char http_content_soap[] = "application/soap+xml";
	static const char http_content_xml[] = "text/xml";

	char		header[1024];
	char		tstr[32];
	const char	*content;
	int		in_len, out_len, header_len;
	time_t		t;

	in_len = recv(client, in, sizeof(in), 0);
	if (in_len <= 0)
		return;

	in[in_len] = 0;
	out_len = sizeof(out);
	if (handle_request("", in, in_len, out, &out_len, 1) == 0) {
		time(&t);
		strftime(tstr, sizeof(tstr), "%a, %d %b %Y %H:%M:%S GMT", gmtime(&t));

		content = memcmp(in, "POST", 4) == 0 ? http_content_soap : http_content_xml;
		header_len = snprintf(header, sizeof(header), http_response, content, tstr, out_len);

		send(client, header, header_len, MSG_MORE);
		send(client, out, out_len, 0); 
	}
}

int udp_receive(int conn, struct sockaddr *from, int *from_len, struct in_addr *to)
{
	char	msg_control[1024], recv_ip[32];
	struct	iovec iovec[1];
	struct	msghdr 	msg;
	struct	cmsghdr* cmsg;
	int	in_len;

	iovec[0].iov_base = in;
	iovec[0].iov_len = sizeof(in);
	msg.msg_name = from;
	msg.msg_namelen = *from_len;
	msg.msg_iov = iovec;
	msg.msg_iovlen = sizeof(iovec) / sizeof(*iovec);
	msg.msg_control = msg_control;
	msg.msg_controllen = sizeof(msg_control);
	msg.msg_flags = 0;

	in_len = recvmsg(conn, &msg, 0);
	if (in_len <= 0)
		return in_len;
	in[in_len] = 0;

	/* Get local IP */
	*recv_ip = 0;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != 0; cmsg = CMSG_NXTHDR(&msg, cmsg))
		if (cmsg->cmsg_level == IPPROTO_IP && cmsg->cmsg_type == IP_PKTINFO) {
			struct in_pktinfo* pi = (struct in_pktinfo*)CMSG_DATA(cmsg);
			*to = pi->ipi_spec_dst;
		}

	*from_len = msg.msg_namelen;
	return in_len;
}

int udp_send(int socket, const struct in_addr from, const struct sockaddr *to, int to_len, int out_len)
{
	char	msg_control[1024];
	struct	iovec iovec[1];
	struct	msghdr 	msg;
	struct	cmsghdr* cmsg;
	struct 	in_pktinfo *pktinfo;

	iovec[0].iov_base = out;
	iovec[0].iov_len = out_len;
	msg.msg_name = (struct sockaddr*)to;
	msg.msg_namelen = to_len;
	msg.msg_iov = iovec;
	msg.msg_iovlen = sizeof(iovec) / sizeof(*iovec);
	msg.msg_control = msg_control;
	msg.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));
	msg.msg_flags = 0;

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = IPPROTO_IP;
	cmsg->cmsg_type = IP_PKTINFO;
	cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
	pktinfo = (struct in_pktinfo*) CMSG_DATA(cmsg);
	pktinfo->ipi_ifindex = 0;
	pktinfo->ipi_spec_dst = from;

	return sendmsg(socket, &msg, 0);
}

int udp_send_all(int socket, struct in_addr *iface, const struct sockaddr* to, int to_len, int out_len)
{
	struct ifconf	ifc;
	char 		buf[1024] = { 0 };
	struct ifreq	*ifr = 0;
	struct in_addr	from;
	int		i;
	int		rc = 0;
	char		ip[64];

	/* Send through all interfaces */
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	ioctl(socket, SIOCGIFCONF, &ifc);
	ifr = ifc.ifc_req;

	for(i=0; i<(ifc.ifc_len/sizeof(*ifr)); i++) {
		from.s_addr = ((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr.s_addr;
		if (iface->s_addr != INADDR_ANY && iface->s_addr != from.s_addr)
			continue;

		if (udp_send(socket, from, to, to_len, out_len) == 1) {
			inet_ntop(AF_INET, &from.s_addr, ip, sizeof(ip));
			wsdd_log(LOG_ERR, "Failed to send udp from %s\n", ip);
			rc = -1;
		}
	}

	return rc;
}

void wsd_udp_request(int conn)
{
	struct	sockaddr from;
	struct	in_addr to;
	int	in_len, out_len, from_len;
	char	recv_ip[32];

	from_len = sizeof(from);
	in_len = udp_receive(conn, &from, &from_len, &to);
	if (in_len <= 0)
		return;

	strncpy(recv_ip, inet_ntoa(to), sizeof(recv_ip));

	out_len = sizeof(out);
	if (handle_request(recv_ip, in, in_len, out, &out_len, 0) == 0)
		udp_send(conn, to, &from, from_len, out_len);
}

/* llmnr responder */
void llmnr_udp_request(int conn)
{
	struct	sockaddr from;
	struct 	in_addr	to;
	int	in_len, out_len, from_len, name_len;

	from_len = sizeof(from);
	in_len = udp_receive(conn, &from, &from_len, &to);

	if (in_len < 13)
		return;

	if (in[3] & 0xf0)	/* check for standard query */
		return;

	name_len = (unsigned char)(in[12]);
	if (name_len >= 0xC0)
		return;

	if (in[12 + name_len + 3] != DNS_TYPE_A)
		return;

	if (strlen(cd_name) != name_len || strncasecmp(cd_name, in+13, name_len))
		return;

	memcpy(out, in, in_len);
	out_len = in_len;
	out[2] = 0x80;		/* response */
	out[7] = 1;		/* one answer */
	
	out[out_len++] = 0xc0;	/* referrral */
	out[out_len++] = 12;	/* first entry */
	out[out_len++] = 0;	/* type A */
	out[out_len++] = DNS_TYPE_A;
	out[out_len++] = 0;	/* class IN */
	out[out_len++] = 1;
	out[out_len++] = 0;	/* TTL */
	out[out_len++] = 0;
	out[out_len++] = 0;
	out[out_len++] = 0;
	out[out_len++] = 0;	/* Address length */
	out[out_len++] = sizeof(to);
	memcpy(out+out_len, &to, sizeof(to));
	out_len += sizeof(to);
	udp_send(conn, to, &from, from_len, out_len);
}

static int set_multicast(int socket, char* maddr, struct in_addr *iface, int action)
{
	struct ifconf	ifc;
	char 		buf[1024] = { 0 };
	struct ifreq	*ifr = 0;
	struct ip_mreq	mreq;
	int		i;
	int		rc = 0;
	char		ip[64];

	mreq.imr_multiaddr.s_addr = inet_addr(maddr);
	mreq.imr_interface = *iface;

	/* For each interface, add to multicast group */
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	ioctl(socket, SIOCGIFCONF, &ifc);
	ifr = ifc.ifc_req;

	for(i=0; i<(ifc.ifc_len/sizeof(*ifr)); i++) {
		mreq.imr_interface.s_addr = ((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr.s_addr;
		if (iface->s_addr != INADDR_ANY && iface->s_addr != mreq.imr_interface.s_addr)
			continue;

		if (setsockopt(socket, IPPROTO_IP, action, &mreq, sizeof(mreq)) < 0) {
			inet_ntop(AF_INET, &mreq.imr_interface.s_addr, ip, sizeof(ip));
			wsdd_log(LOG_ERR, "Failed to set multicast for %s\n", ip);
			rc = 1;
		}
	}

	return rc;
}

static void sigterm_handler(int sig, siginfo_t *siginfo, void *context)
{
	terminate = 1;
}

/* main function */
int main(int argc, char *argv[])
{
	int			opt, retval = 0;
	struct sockaddr_in	si, wsd_mcast;
	uuid_t			uuid;
	char 			uuid_str[37];
	struct sigaction	act;
	static const int 	enable = 1;
	int			out_len;
	int			conn, clients, i, activity;
	struct pollfd		fds[MAX_CLIENTS+3];
	char			iface[32] = "";
	struct 			in_addr iface_addr;

	gethostname(cd_name, sizeof(cd_name));
	iface_addr.s_addr = INADDR_ANY;

	/* process command line options */
	while ((opt = getopt(argc, argv, "dhFIn:w:i:")) != -1) {
		switch (opt) {

		case 'd':
			loglevel = LOG_DEBUG;
			asdaemon = 0;
			usesyslog = 0;
			break;

		case 'F':
			asdaemon = 0;
			break;

		case 'I':
			usesyslog = 0;
			break;

		case 'i':
			strncpy(iface, optarg, sizeof(iface));
			break;

		case 'n':
			strncpy(cd_name, optarg, sizeof(cd_name));
			break;

		case 'w':
			strncpy(cd_workgroup, optarg, sizeof(cd_workgroup));
			break;

		case 'h':
			printf("usage: wsdd [-d (debug)] [-I (interactive) [-F (foreground)]\n"
			       "            [-h] [-n hostname] [-w workgroup] [-i ip]\n");
			return(0);
		}
	}

	if (*iface)
		if (inet_aton(iface, &iface_addr) == 0) {
			wsdd_log(LOG_ERR, "Invalid interface address %s", iface);
			return 1;
		}

	/* Set signal handler for graceful shutdown */
	memset(&act, 0, sizeof(act));
 	act.sa_sigaction = &sigterm_handler;
	act.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	
	if (asdaemon)
		daemonize();

	if (usesyslog)
		openlog("wsdd", LOG_PID, LOG_DAEMON);

	/* Generate UUIDs */
	instance_id = time(NULL);
	uuid_generate_time(uuid);
	uuid_unparse(uuid, uuid_str);
	sprintf(endpoint, "urn:uuid:%s", uuid_str);
	uuid_generate_time(uuid);
	uuid_unparse(uuid, uuid_str);
	sprintf(sequence, "urn:uuid:%s", uuid_str);

	memset((char *) &wsd_mcast, 0, sizeof(wsd_mcast));
	wsd_mcast.sin_family = AF_INET;
	wsd_mcast.sin_port = htons(WSD_PORT);
	if (inet_aton(WSD_MCAST_ADDR, &wsd_mcast.sin_addr) == 0) {
		wsdd_log(LOG_ERR, "inet_aton() failed for wsd multicast address");
		return 1;
	}

	/* Prepare LLMNR socket */
	fds[LLMNR_UDP_SOCK].fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fds[LLMNR_UDP_SOCK].fd == -1) {
		wsdd_log(LOG_ERR, "Failed to create socket, %s", strerror(errno));
		return 1;
	}
	setsockopt(fds[LLMNR_UDP_SOCK].fd, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));

	memset((char *) &si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(LLMNR_PORT);
	si.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(fds[LLMNR_UDP_SOCK].fd, (const struct sockaddr*)&si, sizeof(si)) == -1) {
		wsdd_log(LOG_ERR, "Failed to listen to UDP port %d for LLMNR", ntohs(si.sin_port));
		retval = 1;
		goto llmnr_udp_close;
	}	

	if (set_multicast(fds[LLMNR_UDP_SOCK].fd, LLMNR_MCAST_ADDR, &iface_addr, IP_ADD_MEMBERSHIP)) {
		wsdd_log(LOG_ERR, "Failed to add multicast for LLMNR: %s", strerror(errno));
		retval = 1;
		goto llmnr_drop_multicast;
	}

	fds[WSD_UDP_SOCK].fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (fds[WSD_UDP_SOCK].fd == -1) {
		wsdd_log(LOG_ERR, "Failed to create socket for wsdd");
		retval = 1;
		goto llmnr_drop_multicast;
	}
	setsockopt(fds[WSD_UDP_SOCK].fd, IPPROTO_IP, IP_PKTINFO, &enable, sizeof(enable));

	memset((char *) &si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(WSD_PORT);
	si.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(fds[WSD_UDP_SOCK].fd, (const struct sockaddr*)&si, sizeof(si)) == -1) {
		wsdd_log(LOG_ERR, "Failed to listen to UDP port %d for WSDD", ntohs(si.sin_port));
		retval = 1;
		goto wsd_udp_close;
	}

	if (set_multicast(fds[WSD_UDP_SOCK].fd, WSD_MCAST_ADDR, &iface_addr, IP_ADD_MEMBERSHIP)) {
		wsdd_log(LOG_ERR, "Failed to add multicast for WSDD: %s", strerror(errno));
		retval = 1;
		goto wsd_drop_multicast;
	}

	fds[WSD_HTTP_SOCK].fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fds[WSD_HTTP_SOCK].fd == -1) {
		wsdd_log(LOG_ERR, "Failed to create http socket");
		retval = 1;
		goto wsd_drop_multicast;
	}	
	setsockopt(fds[WSD_HTTP_SOCK].fd, SOL_SOCKET, SO_REUSEADDR, &enable , sizeof(enable));

	memset((char *) &si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(WSD_HTTP_PORT);
	si.sin_addr = iface_addr;
	if (bind(fds[WSD_HTTP_SOCK].fd, (const struct sockaddr*)&si, sizeof(si)) < 0) {
		wsdd_log(LOG_ERR, "Failed to listen to HTTP port %d", WSD_HTTP_PORT);
		retval = 1;
		goto wsd_http_close;
	}	

	if (listen(fds[WSD_HTTP_SOCK].fd, 5) < 0) {
		wsdd_log(LOG_ERR, "Failed to listen to HTTP port %d", WSD_HTTP_PORT);
		retval = 1;
		goto wsd_http_close;
	}

	/* Send hello message */
	out_len = sizeof(out);
	action_hello(out, &out_len, wsd_device, 0);
	if (udp_send_all(fds[WSD_UDP_SOCK].fd, &iface_addr, (const struct sockaddr*)&wsd_mcast, sizeof(wsd_mcast), out_len) == -1) {
		wsdd_log(LOG_ERR, "Failed to send hello with %d", errno);
		retval = 1;
		goto wsd_http_close;
	}

	clients = 0;
	fds[WSD_HTTP_SOCK].events = POLLIN;
	fds[WSD_UDP_SOCK].events = POLLIN;
	fds[LLMNR_UDP_SOCK].events = POLLIN;
	
	while(!terminate) {
		activity = poll(fds , 3 + clients, -1);
		if (activity == -1) {
			if (errno != EINTR)
				wsdd_log(LOG_ERR, "Select failed");
			continue;
		}
		
		if (fds[WSD_HTTP_SOCK].revents & POLLIN) {
			conn = accept(fds[WSD_HTTP_SOCK].fd, NULL, NULL);
			if (conn < 0)
				continue;
			if (clients < MAX_CLIENTS) {
				fds[3+clients].fd = conn;
				fds[3+clients].events = POLLIN;
				fds[3+clients].revents = 0;
				clients++;
			} else
				close(conn);
			--activity;
			if (!activity)
				continue;
		}

		if (fds[WSD_UDP_SOCK].revents & POLLIN) {
			wsd_udp_request(fds[WSD_UDP_SOCK].fd);
			--activity;
			if (!activity)
				continue;
		}

		if (fds[LLMNR_UDP_SOCK].revents & POLLIN) {
			llmnr_udp_request(fds[LLMNR_UDP_SOCK].fd);
			--activity;
			if (!activity)
				continue;
		}

		i = 0;
		while (i<clients && activity) {
			if (fds[3+i].revents & POLLIN) {
				conn = fds[3+i].fd;
				wsdd_http_request(conn);
				shutdown(conn, SHUT_RDWR);
				close(conn);
				--clients;
				--activity;
				fds[3+i] = fds[3+clients];
			} else
				i++;
		}
	}

	/* Send bye message */
	out_len = sizeof(out);
	action_bye(out, &out_len, wsd_device, 0);
	if (udp_send_all(fds[WSD_UDP_SOCK].fd, &iface_addr, (const struct sockaddr*)&wsd_mcast, sizeof(wsd_mcast), out_len) == -1) {
		wsdd_log(LOG_ERR, "Failed to send bye with %d", errno);
		retval = 1;
	}

wsd_http_close:	
	shutdown(fds[WSD_HTTP_SOCK].fd, SHUT_RDWR);
	close(fds[WSD_HTTP_SOCK].fd);

wsd_drop_multicast:
	set_multicast(fds[WSD_UDP_SOCK].fd, WSD_MCAST_ADDR, &iface_addr, IP_DROP_MEMBERSHIP);

wsd_udp_close:
	shutdown(fds[WSD_UDP_SOCK].fd, SHUT_RDWR);
	close(fds[WSD_UDP_SOCK].fd);
	
llmnr_drop_multicast:
	set_multicast(fds[LLMNR_UDP_SOCK].fd, LLMNR_MCAST_ADDR, &iface_addr, IP_DROP_MEMBERSHIP);

llmnr_udp_close:
	shutdown(fds[LLMNR_UDP_SOCK].fd, SHUT_RDWR);
	close(fds[LLMNR_UDP_SOCK].fd);

	if (asdaemon)
		closelog();

	return retval;
}
