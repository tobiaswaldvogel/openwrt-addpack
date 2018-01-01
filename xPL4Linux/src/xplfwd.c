#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "xPL.h"
#include "xplfwd.h"

#include "args.h"
#include "log.h"

#define VERSION "1.0"

static const char xpld_err_param[] = "Error: %s\nAround %s\n";

void dump_fwds()
{
	FORWARD		*fwd;
	TARGET		*target;
	VALUE		*value;

	for (fwd = fwds.entries; fwd < fwds.entries + fwds.count; fwd++) {
		_log(LOG_INFO, "Group [%s-%s.%s]", fwd->id[0], fwd->id[1], fwd->id[2]);
		for (target = fwd->targets; target < fwd->targets + fwd->count; target++) {
			if (!target->repeat_count)
				_log(LOG_INFO, "  Target %s-%s.%s",
					target->id[0], target->id[1], target->id[2]);
			else
				_log(LOG_INFO, "  Target %s-%s.%s (count %d, intervall %d)",
					target->id[0], target->id[1], target->id[2],
					target->repeat_count, target->intervall);

			for (value = target->values; value < target->values + target->count; value++) {
				if (value->value)
					_log(LOG_INFO, "    %s=%s", value->name, value->value);
				else
					_log(LOG_INFO, "    %s * %d / %d", value->name, value->multiplier, value->divisor);
			}
		}
		_log(LOG_INFO, "");
	}
}

char* parse_values(char *arg, TARGET* target)
{
	VALUE	*value;
	size_t	len;

	while (*arg == ',') {
		if (target->count == target->size) {
			VALUE	*arr = target->values;
			int	new_size = target->size + 8;

			target->values = malloc(sizeof(VALUE) * new_size);
			if (arr) {
				memcpy(target->values, arr, sizeof(VALUE) * target->size);
				free(arr);
			}
			target->size = new_size;
		}

		value = target->values + target->count;
		memset(value, 0, sizeof(VALUE));

		arg++;
		for (len = 0; arg[len] && arg[len] != '=' && arg[len] != '/' && arg[len] != '*' && arg[len] != ',' &&  arg[len] != ':'; len++);
		if (!*arg || *arg == ',' || *arg == ':')
			return 0;
		
		value->name = strndup(arg, len);
		arg += len;
		
		/* Multiplier and divisor */
		while (*arg && *arg != ',' && *arg != ':') {
			if (*arg == '*') {
				value->multiplier = strtol(arg + 1, &arg , 0);
				continue;
			}
			if (*arg == '/') {
				value->divisor = strtol(arg + 1, &arg , 0);
				continue;
			}

			arg++;
			for (len = 0; arg[len] && arg[len] != ',' && arg[len] != ':'; len++);

			if (0 == strcmp("count", value->name)) {
				target->repeat_count = atoi(arg);
				free(value->name);
				value->name = 0;
			} else if (0 == strcmp("intervall", value->name)) {
				target->intervall = atoi(arg);
				free(value->name);
				value->name = 0;
			} else
				value->value = strndup(arg, len);

			arg += len;
		}

		if (value->name)
			target->count++;
	}

	return arg;
}

char* parse_id(char* arg, char* id[3])
{
	char	sep;
	size_t	len;
	int	e;

	for (e = 0; e < 3; e++) {
		sep = e == 0 ? '-' : e == 1 ? '.' : ':';

		for (len = 0; arg[len] &&
			      arg[len] != sep &&
			      arg[len] != ',' &&
			      arg[len] != '='; len++);

		id[e] = strndup(arg, len);
		arg += len;

		if (e == 2)
			continue;

		if (!*arg || *arg == ',' || *arg == '=')
			return 0;
		arg++;
	}

	return arg;
}

bool add_fwd(char** args, void *unused)
{
	FORWARD	*fwd;
	TARGET	*target;
	char	*arg, *last_arg;

	if (fwds.count == fwds.size) {
		FORWARD	*arr = fwds.entries;
		int	new_size = fwds.size + 8;

		fwds.entries = malloc(sizeof(FORWARD) * new_size);
		if (arr) {
			memcpy(fwds.entries, arr, sizeof(FORWARD) * fwds.size);
			free(arr);
		}
		fwds.size = new_size;
	}


	fwd = fwds.entries + fwds.count;
	memset(fwd, 0, sizeof(FORWARD));

	last_arg = *args;
	arg = parse_id(last_arg, fwd->id);
	if (!arg || *arg != '=')
		goto cleanup;

	while (*arg) {
		if (fwd->count == fwd->size) {
			TARGET	*arr = fwd->targets;
			int	new_size = fwd->size + 8;

			fwd->targets = malloc(sizeof(TARGET) * new_size);
			if (arr) {
				memcpy(fwd->targets, arr, sizeof(TARGET) * fwd->size);
				free(arr);
			}
			fwd->size = new_size;
		}

		target = fwd->targets + fwd->count;
		memset(target, 0, sizeof(TARGET));

		arg++;
		last_arg = arg;
		arg = parse_id(last_arg, target->id);
		if (!arg)
			goto cleanup;

		if (*arg == ',') {
			last_arg = arg;
			arg = parse_values(last_arg, target);
			if (!arg)
				goto cleanup;
		}

		fwd->count++;
	}

	fwds.count++;
	return true;

cleanup:
	_log(LOG_ERR, xpld_err_param, *args, last_arg);
	return false;
}

void fwd_timerHandler(int sig, siginfo_t *si, void *uc)
{
	SCHEDULED_MSG	*sm = si->si_value.sival_ptr;

	xPL_sendMessage(sm->msg);
	--(sm->repeat_count);

	if (sm->repeat_count == 0) {
		timer_delete(sm->timer_id);
		xPL_releaseMessage(sm->msg);
		free(sm);
	}
}

void fwd_msg_handler(xPL_Service *service, xPL_Message *msg, xPL_ObjectPtr data)
{
	FORWARD			*fwd = data;
	TARGET			*target;
	VALUE			*value;
	xPL_Message		*out_msg;
	xPL_NameValueList	*body;
	xPL_NameValuePair	**nv;
	xPL_Service		src;	// dummy service for setting source
	int			i;
	char			*msg_class;
	char			buffer[256];
	int			pos = 0;

	msg_class = xPL_getSchemaClass(msg);
	if (msg_class && 0 == strcmp("hbeat", msg_class))
		return;

	src.serviceVendor     = xPL_getSourceVendor(msg);
	src.serviceDeviceID   = xPL_getSourceDeviceID(msg);
	src.serviceInstanceID = xPL_getSourceInstanceID(msg);
	body                  = xPL_getMessageBody(msg);

	for (target = fwd->targets; target < fwd->targets + fwd->count; target++) {
		if ((out_msg = xPL_createTargetedMessage(&src, xPL_getMessageType(msg),
		     target->id[0], target->id[1], target->id[2])) == NULL)
			continue;

		pos = sprintf(buffer, "xPL fwd %s-%s.%s -> %s-%s.%s : %s.%s {",
			src.serviceVendor, src.serviceDeviceID, src.serviceInstanceID,
			fwd->id[0], fwd->id[1], fwd->id[2],
			xPL_getSchemaClass(msg), xPL_getSchemaType(msg));

		if (body)
			for (i = 0, nv = body->namedValues; i < body->namedValueCount; i++) {
				xPL_setMessageNamedValue(out_msg, nv[i]->itemName, nv[i]->itemValue);
				pos += sprintf(buffer+pos, " %s=%s", nv[i]->itemName, nv[i]->itemValue);
			}

		pos += sprintf(buffer+pos, " } => %s-%s.%s",
			target->id[0], target->id[1], target->id[2]);

		xPL_setSchema(out_msg, xPL_getSchemaClass(msg), xPL_getSchemaType(msg));

		for (value = target->values; value < target->values + target->count; value++) {
			if (0 == strcmp("svg", value->name))
				continue;

			if (value->value) {
				xPL_setMessageNamedValue(out_msg, value->name, value->value);
				pos += sprintf(buffer+pos, ",%s=%s", value->name, value->value);
			} else {
				char vstr[32];
				int v = strtol(xPL_getMessageNamedValue(out_msg, value->name), 0, 16);
				
				if (value->multiplier)
					v *= value->multiplier;
				if (value->divisor)
					v /= value->divisor;
				
				snprintf(vstr, sizeof(vstr), "%x", v);
				xPL_setMessageNamedValue(out_msg, value->name, vstr);
				pos += sprintf(buffer+pos, ",%s=%s", value->name, vstr);
			}
		}

		xPL_sendMessage(out_msg);

		if (target->repeat_count > 1)
		{
			SCHEDULED_MSG	*sm;
			struct sigevent         te;
			struct itimerspec       its;

			pos += sprintf(buffer+pos, " (count %d, intervall %d ms)",
				target->repeat_count, target->intervall);
			sm = malloc(sizeof(SCHEDULED_MSG));
			sm->repeat_count = target->repeat_count - 1;
			sm->msg = out_msg;

			te.sigev_notify = SIGEV_SIGNAL;
			te.sigev_signo = SIGRTMIN;
			te.sigev_value.sival_ptr = sm;
			timer_create(CLOCK_REALTIME, &te, &(sm->timer_id));

			its.it_interval.tv_sec = target->intervall / 1000;
			its.it_interval.tv_nsec = (target->intervall  % 1000) * 1000000;
			its.it_value.tv_sec = its.it_interval.tv_sec;
			its.it_value.tv_nsec = its.it_interval.tv_nsec;
			timer_settime(sm->timer_id, 0, &its, NULL);
		} else {
			xPL_releaseMessage(out_msg);
		}

		_log(LOG_NOTICE, "%s", buffer);
	}
}
