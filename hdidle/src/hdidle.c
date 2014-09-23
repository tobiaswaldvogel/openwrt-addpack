/*
 * hdidle.c - external disk idle daemon
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <scsi/sg.h>
#include <scsi/scsi.h>

#define STAT_FILE "/proc/diskstats"
#define DEFAULT_IDLE_TIME 600

/* typedefs and structures */
typedef struct IDLE_TIME {
	struct IDLE_TIME	*next;
	char			*name;
	int			idle_time;
} IDLE_TIME;

typedef struct DISKSTATS {
	struct DISKSTATS	*next;
	char			name[50];
	int			idle_time;
	time_t			last_io;
	time_t			spindown;
	time_t			spinup;
	unsigned int		spun_down : 1;
	unsigned int		reads;
	unsigned int		writes;
} DISKSTATS;

/* global/static variables */
IDLE_TIME *it_root;
DISKSTATS *ds_root;
int loglevel = LOG_INFO;
int asdaemon = 1;
int usesyslog = 1;

void write_log(int priority, const char* format, ...) {
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

static char *disk_name(char *path)
{
	ssize_t len;
	char buf[256];
	char *s;

	if (*path != '/')
		return path;

	if ((len = readlink(path, buf, sizeof(buf) - 1)) <= 0) {
		if (errno != EINVAL)
			return path;

		/* 'path' is not a symlink */
		strncpy(buf, path, sizeof(buf) - 1);
		buf[sizeof(buf)-1] = '\0';
		len = strlen(buf);
	}
	buf[len] = '\0';

	/* remove partition numbers, if any */
	for (s = buf + strlen(buf) - 1; s >= buf && isdigit(*s); s--)
		*s = '\0';

	/* Extract basename of the disk in /dev. Note that this assumes that the
	* final target of the symlink (if any) resolves to /dev/sd*
	*/
	if ((s = strrchr(buf, '/')) != NULL)
		s++;
	else
		s = buf;

	if ((s = strdup(s)) == NULL) {
		fprintf(stderr, "out of memory");
		exit(2);
	}

	write_log(LOG_DEBUG, "using %s for %s", s, path);
	return(s);
}

/* print hex dump to stderr (e.g. sense buffers) */
static void phex(const void *p, int len, const char *fmt, ...)
{
	char printbuf[1024];
	int bufpos = 0;

	va_list va;
	const unsigned char *buf = p;
	int pos = 0;
	int i;

	/* print header */
	va_start(va, fmt);
	bufpos += vsprintf(printbuf+bufpos, fmt, va);

	/* print hex block */
	while (len > 0) {
		bufpos += sprintf(printbuf+bufpos, "%08x ", pos);

		/* print hex block */
		for (i = 0; i < 16; i++) {
			if (i < len) {
				bufpos += sprintf(printbuf+bufpos, "%c%02x", ((i == 8) ? '-' : ' '), buf[i]);
			} else {
				bufpos += sprintf(printbuf+bufpos, "   ");
			}
		}

		/* print ASCII block */
		fprintf(stderr, "   ");
		for (i = 0; i < ((len > 16) ? 16 : len); i++) {
			bufpos += sprintf(printbuf+bufpos, "%c", (buf[i] >= 32 && buf[i] < 128) ? buf[i] : '.');
		}

		write_log(LOG_DEBUG, "%s", printbuf);

		pos += 16;
		buf += 16;
		len -= 16;
	}
}

static void spindown_disk(char* name)
{
	struct sg_io_hdr io_hdr;
	unsigned char sense_buf[255];
	char dev_name[100];
	int fd;

	write_log(LOG_INFO, "disk %s: spindown", name);

	/* fabricate SCSI IO request */
	memset(&io_hdr, 0x00, sizeof(io_hdr));
	io_hdr.interface_id = 'S';
	io_hdr.dxfer_direction = SG_DXFER_NONE;

	/* SCSI stop unit command */
	io_hdr.cmdp = (unsigned char *) "\x1b\x00\x00\x00\x00\x00";

	io_hdr.cmd_len = 6;
	io_hdr.sbp = sense_buf;
	io_hdr.mx_sb_len = (unsigned char) sizeof(sense_buf);

	snprintf(dev_name, sizeof(dev_name), "/dev/%s", name);
	if ((fd = open(dev_name, O_RDONLY)) < 0) {
		perror(dev_name);
		return;
	}

	/* execute SCSI request */
	if (ioctl(fd, SG_IO, &io_hdr) < 0) {
		char buf[100];
		snprintf(buf, sizeof(buf), "ioctl on %s:", name);
		perror(buf);

	} else if (io_hdr.masked_status != 0) {
		write_log(LOG_ERR, "error: SCSI command failed with status 0x%02x",
					io_hdr.masked_status);
		if (io_hdr.masked_status == CHECK_CONDITION) {
			phex(sense_buf, io_hdr.sb_len_wr, "sense buffer:\n");
		}
	}

	close(fd);
}

static DISKSTATS *get_diskstats(const char *name)
{
	DISKSTATS *ds;

	for (ds = ds_root; ds != NULL; ds = ds->next)
		if (!strcmp(ds->name, name)) 
			return(ds);

	return NULL;
}

static void daemonize(void)
{
	FILE *fp;
	int maxfd;
	int i;

	if ((i = fork()) < 0) {
		perror("couldn't fork");
		exit(2);
	} else if (i > 0) {
		_exit(0);
	}

	setsid();
	if ((i = fork()) < 0) {
		perror("couldn't fork #2");
		exit(2);
	} else if (i > 0) {
		_exit(0);
	}

	if ((fp=fopen("/var/run/hdidle.pid", "w")) != NULL)
	{
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	chdir("/tmp");
	maxfd = getdtablesize();
	for (i = 0; i < maxfd; i++)
		close(i);

	open("/dev/null", O_RDONLY);
	open("/dev/null", O_WRONLY);
	open("/dev/null", O_WRONLY);
}

int main(int argc, char *argv[])
{
	IDLE_TIME *it;
	int min_idle_time;
	int sleep_time;
	int opt;

	if ((it = malloc(sizeof(*it))) == NULL) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}
	it->next = NULL;
	it->name = NULL;
	it->idle_time = DEFAULT_IDLE_TIME;
	it_root = it;

	while ((opt = getopt(argc, argv, "t:a:i:l:dnhI")) != -1) {
		switch (opt) {

		case 't':
			spindown_disk(optarg);
			return(0);

		case 'a':
			if ((it = malloc(sizeof(*it))) == NULL) {
			fprintf(stderr, "out of memory\n");
			return(2);
			}
			it->name = disk_name(optarg);
			it->idle_time = DEFAULT_IDLE_TIME;
			it->next = it_root;
			it_root = it;
			break;

		case 'i':
			it->idle_time = atoi(optarg);
			break;

		case 'd':
			loglevel = LOG_DEBUG;
			break;

		case 'n':
			asdaemon = 0;
			break;

		case 'I':
			loglevel = LOG_DEBUG;
			usesyslog = 0;
			break;

		case 'h':
			printf("usage: hd-idle [-t <disk>] [-a <name>] [-i <idle_time>] [-d (debug) ]\n"
			       "               [-n (no daemon)] [-I (interactive)] [-h]\n");
			return(0);

		case ':':
			fprintf(stderr, "error: option -%c requires an argument\n", optopt);
			return(1);

		case '?':
			fprintf(stderr, "error: unknown option -%c\n", optopt);
			return(1);
		}
	}

	/* set sleep time to 1/10th of the shortest idle time */
	min_idle_time = 1 << 30;
	for (it = it_root; it != NULL; it = it->next) {
		if (it->idle_time != 0 && it->idle_time < min_idle_time) {
			min_idle_time = it->idle_time;
		}
	}
	if ((sleep_time = min_idle_time / 10) == 0) {
		sleep_time = 1;
	}

	if (asdaemon)
		daemonize();

	if (usesyslog)
		openlog("hdidle", LOG_PID, LOG_DAEMON);

	for (;;) {
		DISKSTATS tmp;
		char buf[200];
		FILE *fp;

		if ((fp = fopen(STAT_FILE, "r")) == NULL) {
			perror(STAT_FILE);
			return(2);
		}

		memset(&tmp, 0x00, sizeof(tmp));

		while (fgets(buf, sizeof(buf), fp) != NULL) {
			if (!sscanf(buf, "%*d %*d %s %*u %*u %u %*u %*u %*u %u %*u %*u %*u %*u",
					tmp.name, &tmp.reads, &tmp.writes) == 3)
				continue;

			DISKSTATS *ds;
			time_t now = time(NULL);

			/* make sure this is a SCSI disk (sd[a-z]) */
			if (tmp.name[0] != 's' || tmp.name[1] != 'd' ||
				!isalpha(tmp.name[2]) || tmp.name[3] != '\0')
				continue;

			write_log(LOG_DEBUG, "probing %s: reads: %u, writes: %u",
					tmp.name, tmp.reads, tmp.writes);

			/* get previous statistics for this disk */
			ds = get_diskstats(tmp.name);

			if (ds == NULL) {
			/* new disk; just add it to the linked list */
				if ((ds = malloc(sizeof(*ds))) == NULL) {
					fprintf(stderr, "out of memory\n");
					return(2);
				}
				memcpy(ds, &tmp, sizeof(*ds));
				ds->last_io = now;
				ds->spinup = ds->last_io;
				ds->next = ds_root;
				ds_root = ds;

				/* find idle time for this disk (falling-back to default; default means
				* 'it->name == NULL' and this entry will always be the last due to the
				* way this single-linked list is built when parsing command line
				* arguments)
				*/
				for (it = it_root; it != NULL; it = it->next)
					if (it->name == NULL || !strcmp(ds->name, it->name)) {
						ds->idle_time = it->idle_time;
						write_log(LOG_INFO, "disk %s: standby timeout %u seconds",
									ds->name, ds->idle_time);
						break;
					}
				continue;
			}

			/* no activity */
			if (ds->reads == tmp.reads && ds->writes == tmp.writes) {
				if (ds->spun_down)
					continue;

				/* no activity on this disk and still running */
				if (ds->idle_time != 0 && now - ds->last_io >= ds->idle_time) {
					spindown_disk(ds->name);
					ds->spindown = now;
					ds->spun_down = 1;
				}
				continue;
			}


			ds->reads = tmp.reads;
			ds->writes = tmp.writes;
			ds->last_io = now;
			ds->spun_down = 0;
			if (!ds->spun_down)
				continue;

			/* disk was spun down, thus it has just spun up */
			write_log(LOG_INFO, "disk %s: spinup (running: %ld, stopped: %ld)",
			ds->name,
			(long) ds->spindown - (long) ds->spinup,
			(long) time(NULL) - (long) ds->spindown);
			ds->spinup = now;
		}

		fclose(fp);
		sleep(sleep_time);
	}

	return(0);
}
