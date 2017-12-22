#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "log.h"

int	loglevel = LOG_NOTICE;
int	usesyslog = 0;

void _log(int priority, const char* format, ...) {
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
