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
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <time.h>
#include <pthread.h>
#include <uuid/uuid.h>
#include <libubus.h>

#define WSD_PORT (3702)
#define WSD_MCAST_ADDR ("239.255.255.250")

static const char wsd_hello[] = "Hello";
static const char wsd_bye[] = "Bye";
static const char wsd_resolve[] = "Resolve";
static const char wsd_act_prefix[] = "http://schemas.xmlsoap.org/ws/2005/04/discovery/";
static const char wsd_header[] =
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
  "<soap:Envelope "
     "xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\" "
     "xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
     "xmlns:wsd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
     "xmlns:wsdp=\"http://schemas.xmlsoap.org/ws/2006/02/devprof\" "
     "xmlns:pub=\"http://schemas.microsoft.com/windows/pub/2005/07\">"
    "<soap:Header>";
static const char wsd_msg[] =
  "%s"
      "<wsa:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
      "<wsa:Action>%s%s</wsa:Action>"
      "<wsa:MessageID>urn:uuid:%s</wsa:MessageID>"
      "<wsd:AppSequence InstanceId=\"%d\" SequenceId=\"urn:uuid:%s\" MessageNumber=\"%d\" />"
    "</soap:Header>"
    "<soap:Body>"
      "<wsd:%s>"
        "<wsa:EndpointReference>"
        "<wsa:Address>%s</wsa:Address>"
        "</wsa:EndpointReference>"
        "<wsd:Types>%s</wsd:Types>"
        "<wsd:MetadataVersion>2</wsd:MetadataVersion>"
      "</wsd:%s>"
    "</soap:Body>"
  "</soap:Envelope>";
static const char wsd_match[] =
  "%s"
      "<wsa:To>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To>"
      "<wsa:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/ResolveMatches</wsa:Action>"
      "<wsa:MessageID>urn:uuid:%s</wsa:MessageID>"
      "<wsa:RelatesTo>%s</wsa:RelatesTo>"
      "<wsd:AppSequence InstanceId=\"%d\" SequenceId=\"urn:uuid:%s\" MessageNumber=\"%d\" />"
   "</soap:Header>"
   "<soap:Body>"
      "<wsd:ResolveMatches>"
         "<wsd:ResolveMatch>"
            "<wsa:EndpointReference>"
               "<wsa:Address>%s</wsa:Address>"
            "</wsa:EndpointReference>"
            "<wsd:Types>wsdp:Device pub:Computer</wsd:Types>"
            "<wsd:XAddrs>http://192.168.3.32/wsd/</wsd:XAddrs>"
            "<wsd:MetadataVersion>2</wsd:MetadataVersion>"
         "</wsd:ResolveMatch>"
      "</wsd:ResolveMatches>"
    "</soap:Body>"
  "</soap:Envelope>";

static const char wsd_addr[]  = ":Address>";
static const char wsd_msgid[] = ":MessageID>";
static const char wsd_body[]  = ":Body>";

/* function prototypes */
void	daemonize(void);
void	wsdd_log(int priority, const char* format, ...);
int	wsdd_ubus_init(const char *path);

/* global/static variables */
int loglevel = LOG_INFO;
int asdaemon = 1;

int wsd_sock;
struct sockaddr_in mcast_addr;
char endpoint[48];
int instance_id;

static struct ubus_context *ctx = NULL;
static const char *ubus_path;

// ubus call wsdd add '{ "service": "wsdp:Device pub:Computer" }'

static const struct blobmsg_policy wsdd_policy[] = {
	[0] = { .name = "service", .type = BLOBMSG_TYPE_STRING },
};

static int
wsdd_handle_add(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[1];
	char *service;
	char buffer[8192];
	int len;
	char app_seq[37], msg_id[37];
        uuid_t uu;

	wsdd_log(LOG_INFO, "add");
	blobmsg_parse(wsdd_policy, ARRAY_SIZE(wsdd_policy), tb, blob_data(msg), blob_len(msg));
	
	if (!tb[0])
		return 0;
	
	service = blobmsg_data(tb[0]);

	wsdd_log(LOG_INFO, "Received %s %s", method, service);

	uuid_generate_time(uu);
	uuid_unparse(uu, app_seq);
	uuid_generate_time(uu);
	uuid_unparse(uu, msg_id);
	len = snprintf(buffer, sizeof(buffer), wsd_msg, wsd_header, wsd_act_prefix, wsd_hello, msg_id, instance_id, app_seq, 1, wsd_hello, endpoint, service, wsd_hello);
	
	if (sendto(wsd_sock, buffer, len, 0, (const struct sockaddr*)&mcast_addr, sizeof(mcast_addr)) == -1)
		wsdd_log(LOG_ERR, "sendto() failed");
	return 0;
}

static int
wsdd_handle_remove(struct ubus_context *ctx, struct ubus_object *obj,
		     struct ubus_request_data *req, const char *method,
		     struct blob_attr *msg)
{
	struct blob_attr *tb[1];
	char *msgstr = "(unknown)";
	char buffer[8192];
	int len;
	char uu_str[37];
        uuid_t uu;
	
	wsdd_log(LOG_INFO, "add");
	blobmsg_parse(wsdd_policy, ARRAY_SIZE(wsdd_policy), tb, blob_data(msg), blob_len(msg));
	
	if (tb[0])
		msgstr = blobmsg_data(tb[0]);

	wsdd_log(LOG_INFO, "Received %s %s", method, msgstr);

	uuid_generate_time(uu);
	uuid_unparse(uu, uu_str);
	len = snprintf(buffer, sizeof(buffer), wsd_msg, wsd_bye, uu_str, wsd_bye, endpoint, "wsdp:Device pub:Computer", wsd_bye);

	if (sendto(wsd_sock, buffer, len, 0, (const struct sockaddr*)&mcast_addr, sizeof(mcast_addr)) == -1)
		wsdd_log(LOG_ERR, "sendto() failed");

	wsdd_log(LOG_INFO, buffer);
	return 0;
}

static struct ubus_method main_object_methods[] = {
	UBUS_METHOD("add",     wsdd_handle_add,    wsdd_policy),
	UBUS_METHOD("remove",  wsdd_handle_remove, wsdd_policy),
};

static struct ubus_object_type main_object_type =
	UBUS_OBJECT_TYPE("wsdd", main_object_methods);

static struct ubus_object main_object = {
	.name = "wsdd",
	.type = &main_object_type,
	.methods = main_object_methods,
	.n_methods = ARRAY_SIZE(main_object_methods),
};

void wsdd_ubus_add_fd(void)
{
	ubus_add_uloop(ctx);
#ifdef FD_CLOEXEC
	fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC);
#endif
}

void wsdd_ubus_reconnect_timer(struct uloop_timeout *timeout)
{
	static struct uloop_timeout retry = {
		.cb = wsdd_ubus_reconnect_timer,
	};
	int t = 2;

	if (ubus_reconnect(ctx, ubus_path) != 0) {
		wsdd_log(LOG_INFO, "failed to reconnect, trying again in %d seconds", t);
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

	wsdd_log(LOG_INFO, "reconnected to ubus, new id: %08x", ctx->local_id);
	wsdd_ubus_add_fd();
}

void wsdd_ubus_connection_lost(struct ubus_context *ctx)
{
	wsdd_ubus_reconnect_timer(NULL);
}

int wsdd_ubus_init(const char *path)
{
	int ret;

	uloop_init();
	ubus_path = path;

	ctx = ubus_connect(path);
	if (!ctx)
		return -EIO;

	wsdd_log(LOG_DEBUG, "connected as %08x", ctx->local_id);
	ctx->connection_lost = wsdd_ubus_connection_lost;
	wsdd_ubus_add_fd();

	ret = ubus_add_object(ctx, &main_object);
	if (ret != 0)
		wsdd_log(LOG_INFO, "Failed to publish object: %s", ubus_strerror(ret));

	ubus_add_uloop(ctx);
	return ret;
}

/* become a daemon */
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
	chdir("/");
	maxfd = getdtablesize();
	for (i = 0; i < maxfd; i++) {
		close(i);
	}

	/* use /dev/null for stdin, stdout and stderr */
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_WRONLY);
	open("/dev/null", O_WRONLY);

	openlog("wsdd", LOG_PID, LOG_DAEMON);
}

void wsdd_log(int priority, const char* format, ...) {
	va_list va;
	char printbuf[1024];

	if (priority > loglevel)
		return;

	va_start(va, format);
	vsnprintf(printbuf, sizeof(printbuf), format, va);

	if (asdaemon)
		syslog(priority, "%s", printbuf);
	else
		fprintf(stderr, "%s\n", printbuf);
}

char* get_tag_value(char *xml, const char *tag, unsigned int taglen, unsigned int *len)
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

void match_resolve(const struct sockaddr* sa, unsigned int sa_len, char *src_msgid, char *body)
{
	char	*endp;
	unsigned int endp_len, len;
	char app_seq[37], msg_id[37];
        uuid_t uu;
	char buffer[8192];
	
	endp = get_tag_value(body, wsd_addr, sizeof(wsd_addr)-1, &endp_len);
	if (!endp) {
		wsdd_log(LOG_INFO, "Endpoint not found");
		return;
	}			
	endp[endp_len] = 0;
	
	if (strcasecmp(endp, endpoint)) {
		wsdd_log(LOG_INFO, "Not my endpoint");
		return;
	}

	uuid_generate_time(uu);
	uuid_unparse(uu, app_seq);
	uuid_generate_time(uu);
	uuid_unparse(uu, msg_id);
	len = snprintf(buffer, sizeof(buffer), wsd_match, wsd_header, msg_id, src_msgid, instance_id, app_seq, 1, endpoint);
	
	if (sendto(wsd_sock, buffer, len, 0, sa, sa_len) == -1)
		wsdd_log(LOG_ERR, "sendto() failed");
}

void *udp_server_thread(void *data)
{
	char	buffer[8192];
	struct	sockaddr_in si;
	int	read, si_len = sizeof(si);
	char	*action, *msgid, *body;
	unsigned int action_len, msgid_len;
	
	while ((read = recvfrom(wsd_sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&si, &si_len)) > 0) {
		si_len = sizeof(si);
		buffer[read] = 0;

		action = get_tag_value(buffer, wsd_act_prefix, sizeof(wsd_act_prefix)-1, &action_len);
		if (!action) {
			wsdd_log(LOG_INFO, "Received non wsd message");
			continue;
		}

		msgid = get_tag_value(buffer, wsd_msgid, sizeof(wsd_msgid)-1, &msgid_len);
		if (!msgid) {
			wsdd_log(LOG_INFO, "Msg ID not found");
			continue;
		}

		body = strstr(buffer, wsd_body);
		if (!msgid) {
			wsdd_log(LOG_INFO, "Body not found");
			continue;
		}

		action[action_len] = 0;
		msgid[msgid_len] = 0;
		
		wsdd_log(LOG_INFO, "Received %s from %s:%d", action, inet_ntoa(si.sin_addr), ntohs(si.sin_port));
		if (strcmp(action, wsd_resolve) == 0) {
			match_resolve((struct sockaddr*)&si, si_len, msgid, body);
		}
	}
}

/* main function */
int main(int argc, char *argv[])
{
	const char *ubus_path = NULL;
	struct sockaddr_in si;
	struct ip_mreq imr;
	int opt, retval = 0;
        uuid_t uuid;
	char uuid_str[37];
	pthread_t udp_server = 0;

	instance_id = time(NULL);
/*
	IDLE_TIME *it;
	int min_idle_time;
	int sleep_time;

	if ((it = malloc(sizeof(*it))) == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	it->next = NULL;
	it->name = NULL;
	it->idle_time = DEFAULT_IDLE_TIME;
	it_root = it; 
*/
	/* process command line options */
	while ((opt = getopt(argc, argv, "s:dh")) != -1) {
		switch (opt) {

		case 'd':
			loglevel = LOG_DEBUG;
			asdaemon = 0;
			break;

		case 'h':
			printf("usage: wsdd [-d] [-h]\n");
			return(0);

		case 's':
			ubus_path = optarg;
			break;
		}
	}

	/* daemonize unless we're running in debug mode */
	if (asdaemon)
		daemonize();

	
	uuid_generate_time(uuid);
	uuid_unparse(uuid, uuid_str);
	sprintf(endpoint, "urn:uuid:%s", uuid_str);
	wsdd_log(LOG_DEBUG, "Endpoint %s", endpoint);
	
	memset((char *) &mcast_addr, 0, sizeof(mcast_addr));
	mcast_addr.sin_family = AF_INET;
	mcast_addr.sin_port = htons(WSD_PORT);
	if (inet_aton(WSD_MCAST_ADDR, &mcast_addr.sin_addr)==0) {
		wsdd_log(LOG_ERR, "inet_aton() failed for wsd multicast address");
		return 1;
	}

	wsd_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (wsd_sock == -1) {
		wsdd_log(LOG_ERR, "Failed to create socket");
		return 1;
	}

	imr.imr_multiaddr.s_addr = inet_addr(WSD_MCAST_ADDR);
	imr.imr_interface.s_addr = INADDR_ANY;
	if (setsockopt(wsd_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&imr, sizeof(struct ip_mreq)) < 0)
	{
		wsdd_log(LOG_ERR, "Failed to add multicast membership");
		retval = 1;
		goto close_socket;
	}

	memset((char *) &si, 0, sizeof(si));
	si.sin_family = AF_INET;
	si.sin_port = htons(WSD_PORT);
	si.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(wsd_sock, (const struct sockaddr*)&si, sizeof(si)) == -1) {
		wsdd_log(LOG_ERR, "Failed to listen to UDP port %d", WSD_PORT);
		retval = 1;
		goto drop_multicast;
	}	

	if (pthread_create(&udp_server, NULL, udp_server_thread, NULL)) {
		wsdd_log(LOG_ERR, "Failed to create udp server thread");
		retval = 1;
		goto drop_multicast;
	}	

	if (wsdd_ubus_init(ubus_path) < 0) {
		wsdd_log(LOG_ERR, "Failed to connect to ubus");
		retval = 1;
		goto drop_multicast;
	}
		
	wsdd_log(LOG_INFO, "wsdd running 1");

	uloop_run();

	ubus_free(ctx);
	uloop_done();

drop_multicast:	
	setsockopt(wsd_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *)&imr, sizeof(struct ip_mreq));

close_socket:
	shutdown(wsd_sock, SHUT_RDWR);
	close(wsd_sock);	

	if (udp_server) {
		wsdd_log(LOG_INFO, "Waiting for udp server thread");
		pthread_join(udp_server, NULL);
	}
	
	if (asdaemon)
		closelog();
	return retval;
}
