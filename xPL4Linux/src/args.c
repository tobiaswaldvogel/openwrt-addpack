#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "args.h"
#include "log.h"

bool include_params(char** args, void *unused);
bool help(char** unused, void *data);

bool set_use_syslog(char** unused, void *data)
{
	*((int*)data) = 1;
	openlog(0, 0, LOG_DAEMON);
	return true;
}

bool set_int(char** args, void *data)
{
	*((int*)data) = atoi(*args);
	return true;
}

PARAM_INFO	param_info;

static const PARAM	params_generic[] = {
	{ "i", "include", "Include parameters from file", 1, include_params, &param_info },
	{ "h", "help", "Display this help", 0, help, &param_info },
	{ "s", 0, "Use syslog", 0, set_use_syslog, &usesyslog },
	{ "l", "loglevel", "Set log level (0-7, default 5)", 1, set_int, &loglevel },
};

bool help(char** unused, void *data)
{
	PARAM_INFO	*pi = data;
	char		buffer[1024];
	const PARAM	*p;
	int		s, i;

	sprintf(buffer, "Usage: %s\n", pi->cmdline);
	_log(LOG_ERR, buffer);

	for (s = 0; s < 2; s++) {
		p = s == 0 ? pi->params : params_generic;
		i = s == 0 ? pi->count  : sizeof(params_generic)/sizeof(params_generic[0]);

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
	return true;
}

static const PARAM	params[];

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

bool include_params(char** args, void *data)
{
	PARAM_INFO	*pi = data;
	const PARAM	*params = pi->params;
	int		count   = pi->count;

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
			if (!param->func(parg+1, param->data))
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

	param_info.params = params;
	param_info.count  = count;
	param_info.cmdline = *argv;

	argc--;
	argv++;

	while (true) {
		if (param && pargc > param->args ) {
			if (!param->func(parg+1, param->data))
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

	help(0, &param_info);
	return false;
}
