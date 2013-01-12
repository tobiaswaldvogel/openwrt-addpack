#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <syslog.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>

#include <libubus.h>

/* function prototypes */
void	daemonize(void);
void	wsdd_log(int priority, const char* format, ...);
int	wsdd_ubus_init(const char *path);
void	wsdd_ubus_done(void);

/* global/static variables */
int loglevel = LOG_INFO;
int asdaemon = 1;

static struct ubus_context *ctx = NULL;
static const char *ubus_path;

/* main function */
int main(int argc, char *argv[])
{
	const char *socket = NULL;
	int opt;
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
			socket = optarg;
			break;
		}
	}

	/* daemonize unless we're running in debug mode */
	if (asdaemon)
		daemonize();

	if (wsdd_ubus_init(socket) < 0) {
		wsdd_log(LOG_ERR, "Failed to connect to ubus\n");
		return 1;
	}
		
	wsdd_log(LOG_INFO, "wsdd running\n");
	uloop_run();

	wsdd_ubus_done();

	if (asdaemon)
		closelog();
	return(0);
}

static const struct blobmsg_policy wsdd_policy[] = {
	[0] = { .name = "service", .type = BLOBMSG_TYPE_STRING },
};

static int
wsdd_handle_add(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method,
		struct blob_attr *msg)
{
	struct blob_attr *tb[1];
	char *msgstr = "(unknown)";
	
	wsdd_log(LOG_INFO, "add\n");
	blobmsg_parse(wsdd_policy, ARRAY_SIZE(wsdd_policy), tb, blob_data(msg), blob_len(msg));
	
	if (tb[0])
		msgstr = blobmsg_data(tb[0]);

	wsdd_log(LOG_INFO, "Received %s %s\n", method, msgstr);
	return 0;
}

static int
wsdd_handle_remove(struct ubus_context *ctx, struct ubus_object *obj,
		     struct ubus_request_data *req, const char *method,
		     struct blob_attr *msg)
{
	struct blob_attr *tb[1];
	char *msgstr = "(unknown)";
	
	wsdd_log(LOG_INFO, "remove\n");
	blobmsg_parse(wsdd_policy, ARRAY_SIZE(wsdd_policy), tb, blob_data(msg), blob_len(msg));
	
	if (tb[0])
		msgstr = blobmsg_data(tb[0]);

	wsdd_log(LOG_INFO, "Received %s %s\n", method, msgstr);
	return 0;
}

static struct ubus_method main_object_methods[] = {
	UBUS_METHOD("add",     wsdd_handle_add, wsdd_policy),
	UBUS_METHOD("remove",  wsdd_handle_add, wsdd_policy),
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
		wsdd_log(LOG_INFO, "failed to reconnect, trying again in %d seconds\n", t);
		uloop_timeout_set(&retry, t * 1000);
		return;
	}

	wsdd_log(LOG_INFO, "reconnected to ubus, new id: %08x\n", ctx->local_id);
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

	wsdd_log(LOG_DEBUG, "connected as %08x\n", ctx->local_id);
	ctx->connection_lost = wsdd_ubus_connection_lost;
	wsdd_ubus_add_fd();

	ret = ubus_add_object(ctx, &main_object);
	if (ret != 0)
		wsdd_log(LOG_INFO, "Failed to publish object: %s\n", ubus_strerror(ret));
	return ret;
}

void wsdd_ubus_done(void)
{
	ubus_free(ctx);
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
