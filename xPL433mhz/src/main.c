#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

extern int xpl433mhz_main(int argc, char **argv);
extern int xpld_main(int argc, char **argv);

static struct {
	const char *name;
	int (*func_main)(int argc, char **argv);
} multicall[] = {
	{ "xpl433mhz", xpl433mhz_main },
	{ "xpld", xpld_main },
};

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int main(int argc, char **argv)
{
	unsigned int i, len;
	extern char *__progname;
	char path[PATH_MAX], *self;

	len = readlink("/proc/self/exe", path, sizeof(path));
	path[len] = 0;
	for (i=len-2; i>0 && path[i] != '/'; --i);
	self = path + i + 1;

	if (0 == strcmp(__progname, self)) {
		if (argc == 1) goto usage;
		argv++;
		--argc;
		__progname = argv[0];
	}

	for (i = 0; i < ARRAY_SIZE(multicall); i++) {
		if (0 == strcmp(__progname, multicall[i].name)) {
			return multicall[i].func_main(argc, argv);
		}
	}

usage:
	fprintf(stderr, "%s multicall binary\nAvailable commands:\n\n", self);
	for (i = 0; i < ARRAY_SIZE(multicall); i++) {
		fprintf(stderr, "%s", multicall[i].name);
		if (i != ARRAY_SIZE(multicall)-1)
			fprintf(stderr, ", ");
	}
	fprintf(stderr, "\n");

	return 1;
}
