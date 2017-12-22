#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>

#include "args.h"

static	int	loglevel = LOG_INFO;
static	int	usesyslog = 0;

bool include_params(char** args);

bool set_use_syslog(char** args)
{
	usesyslog = 1;
	return true;
}

bool set_log_level(char** args)
{
	loglevel = atoi(*args);
	return true;
}


bool return_false(char** args)
{
	return false;
}

static const PARAM	params_generic[] = {
	{ "i", "include", "Include parameters from file", 1, include_params },
	{ "h", "help", "Display this help", 0, return_false },
	{ "s", 0, "Use syslog", 0, set_use_syslog },
	{ "l", "loglevel", "Set log level", 1, set_log_level },
};

static const PARAM	params[];
static const int	params_count;

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

void help(const PARAM *params, int count)
{
	char		buffer[1024];
	const PARAM	*p;
	int		s, i;

	sprintf(buffer, "Usage:\n");
	_log(LOG_ERR, buffer);

	for (s = 0; s < 2; s++) {
		p = s == 0 ? params : params_generic;
		i = s == 0 ? count  : sizeof(params_generic)/sizeof(params_generic[0]);

		while (i > 0) {
			int	pos = 0;

			--i;
			while (pos < 5)
				buffer[pos++] = ' ';
			if (p->name_short)
				pos += sprintf(buffer+pos, "-%s", p->name_short);
			if (p->name_short && p->name_long) {
				buffer[pos++] = ',';
				buffer[pos++] = ' ';
			}
			if (p->name_long)
				pos += sprintf(buffer+pos, "-%s", p->name_long);
			while (pos < 25)
				buffer[pos++] = ' ';
			strcpy(buffer + pos, p->help);
			_log(LOG_ERR, buffer);
			p++;
		}
	}

	_log(LOG_ERR, "");
}

const PARAM* find_param(const PARAM *params, int count, char* name)
{
	const PARAM	*p;
	int		s, i;

	for (s = 2; s > 0; --s) {
		p = s == 2 ? params : params_generic;
		i = s == 2 ?  count : sizeof(params_generic)/sizeof(params_generic[0]);

		for (; i > 0; --i) {
			if ((p->name_short && 0 == strcmp(p->name_short, name)) ||
			    (p->name_long  && 0 == strcmp(p->name_long,  name)))
				return p;
			p++;
		}
	}

	return 0;
}

bool include_params(char** args)
{
	const PARAM	*params = (const PARAM*)args[1];
	int		count = (int)args[2];

	FILE		*file = 0;
	const PARAM	*param = 0;
	char		*parg[4];
	int		pargc = 0;
	char		buffer[4096], *c = 0;
	int		rc;
	bool		success = false;

	file = fopen(*args, "r");
	if (!file) {
		_log(LOG_ERR, "Could not open %s", *args);
		return false;
	}

	while (true) {
		if (param && pargc > param->args ) {
			parg[pargc]   = (char*)params;
			parg[pargc+1] = (char*)count;

			if (!param->func(parg+1))
				break;

			param = 0;
			pargc = 0;
		}


		if (pargc == 0)
			c = buffer;

		while ((rc = fread(c , 1, 1, file)) && *c <= ' ');
		if (!rc) {
			/* EOF */
			success = pargc == 0;
			break;
		}

		parg[pargc++] = c;
		c++;
		while (c < buffer + sizeof(buffer) && (rc = fread(c , 1, 1, file)) && *c > ' ')
			c++;

		*c = 0;
		c++;

		if (param)
			continue;

		if (!(param = find_param(params, count, *parg)))
			break;
	}

	fclose(file);
	return success;
}

bool parse_args(int argc, char *argv[], const PARAM *params, int count)
{
	const PARAM	*param = 0;
	char		*parg[4];
	int		pargc = 0;

	argc--;
	argv++;

	while (true) {
		if (param && pargc > param->args ) {
			parg[pargc]   = (char*)params;
			parg[pargc+1] = (char*)count;

			if (!param->func(parg+1))
				break;

			param = 0;
			pargc = 0;
		}

		if (!argc)
			return true;

		parg[pargc] = *(argv++);
		if (pargc == 0) {
			if (*parg[pargc] != '-')
				break;
			parg[pargc]++;
		}

		pargc++;
		--argc;

		if (param)
			continue;

		if (!(param = find_param(params, count, *parg)))
			break;
	}

	help(params, count);
	return false;
}

