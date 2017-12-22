#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "xPL.h"

#include "args.h"
#include "log.h"

#define VERSION "1.0"

typedef struct {
	char*		name;
	char*		value;
} VALUE;

typedef struct {
	char*		id[3];
	VALUE		*values;
	int		count;
	int		size;
	int		repeat_count;
	int		intervall;
} TARGET;

typedef struct {
	xPL_Service	*service;
	char*		id[3];
	TARGET		*targets;
	int		count;
	int		size;
} FORWARD;

typedef struct {
	FORWARD		*entries;
	int		count;
	int		size;
} FORWARDS;

typedef struct {
	int		repeat_count;
	int		intervall;
	xPL_Message	*msg;
} SCHEDULED_MSG;

static const char xpld_err_param[] = "Error: %s\nAround %s\n";

FORWARDS	fwds = { 0, 0, 0 };

void dump_fwds()
{
	FORWARD		*fwd;
	TARGET		*target;
	VALUE		*value;

	for (fwd = fwds.entries; fwd < fwds.entries + fwds.count; fwd++) {
		_log(LOG_INFO, "Group [%s-%s.%s]", fwd->id[0], fwd->id[1], fwd->id[2]);
		for (target = fwd->targets; target < fwd->targets + fwd->count; target++) {
			if (!target->repeat_count)
				_log(LOG_INFO, "  Target: %s-%s.%s",
					target->id[0], target->id[1], target->id[2]);
			else
				_log(LOG_INFO, "  Target: %s-%s.%s (count %d, intervall %d)",
					target->id[0], target->id[1], target->id[2],
					target->repeat_count, target->intervall);
			for (value = target->values; value < target->values + target->count; value++)
				_log(LOG_INFO, "    %s=%s", value->name, value->value);
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
		for (len = 0; arg[len] && arg[len] != '='; len++);
		value->name = strndup(arg, len);
		arg += len;
		if (*arg != '=')
			return 0;

		arg++;
		for (len = 0; arg[len] && arg[len] != ',' && arg[len] != ':'; len++);
		value->value = strndup(arg, len);
		arg += len;

		if (0 == strcmp("count", value->name)) {
			target->repeat_count = atoi(value->value);
			free(value->name);
			free(value->value);
		} else if (0 == strcmp("intervall", value->name)) {
			target->intervall = atoi(value->value);
			free(value->name);
			free(value->value);
		} else
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

void *schedule_msg(void * data)
{
	SCHEDULED_MSG	*sm = data;
	int		count;

	xPL_sendMessage(sm->msg);
	for (count = 1; count < sm->repeat_count; count++) {
		usleep(sm->intervall * 1000);
		xPL_sendMessage(sm->msg);
	}

	xPL_releaseMessage(sm->msg);
	free(sm);
	return 0;
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

		pos = sprintf(buffer, "%s-%s.%s -> %s-%s.%s : %s.%s {",
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

			xPL_setMessageNamedValue(out_msg, value->name, value->value);
			pos += sprintf(buffer+pos, ",%s=%s", value->name, value->value);
		}


		if (target->repeat_count)
		{
			pthread_t	thread;
			SCHEDULED_MSG	*sm;

			pos += sprintf(buffer+pos, " (count %d, intervall %d ms)",
				target->count, target->intervall);
			sm = malloc(sizeof(SCHEDULED_MSG));
			sm->repeat_count = target->repeat_count;
			sm->intervall    = target->intervall;
			sm->msg          = out_msg;
			pthread_create(&thread, 0, schedule_msg, sm);
		} else {
			xPL_sendMessage(out_msg);
			xPL_releaseMessage(out_msg);
		}

		_log(LOG_NOTICE, "%s", buffer);
	}
}

void start_fwds()
{
	xPL_ServicePtr	service;
	FORWARD		*fwd;

	for (fwd = fwds.entries; fwd < fwds.entries + fwds.count; fwd++) {
		service =  xPL_createService(fwd->id[0], fwd->id[1], fwd->id[2]);
		xPL_setServiceVersion(service, VERSION);
		xPL_setServiceEnabled(service, TRUE);
		xPL_addServiceListener(service, fwd_msg_handler, xPL_MESSAGE_ANY, NULL, NULL, fwd);
		fwd->service =  service;
	}
}

void xpld_shutdownHandler(int onSignal) {
	FORWARD		*fwd;

	for (fwd = fwds.entries; fwd < fwds.entries + fwds.count; fwd++) {
		if (!fwd->service)
			continue;

		xPL_setServiceEnabled(fwd->service, FALSE);
		xPL_releaseService(fwd->service);
	}

	xPL_shutdown();
	_log(LOG_NOTICE, "Shutdown");
	exit(0);
}

int xpld_main(int argc, String argv[]) {
	static const PARAM	params[] = {
		{ "f", "fwd", "Add fwd", 1, add_fwd },
	};

	if (!xPL_parseCommonArgs(&argc, argv, FALSE)) {
		_log(LOG_EMERG, "Unable to start xPL");
		return -1;
	}

	if (!parse_args(argc, argv, params, sizeof(params)/sizeof(params[0])))
		return -2;

	if (!fwds.count)
		return 0;

	/* Startup xPL */
	if (!xPL_initialize(xPL_getParsedConnectionType())) {
		_log(LOG_EMERG, "Unable to start xPL");
		return -3;
	}

	dump_fwds();
	start_fwds();

	/* Install signal traps for proper shutdown */
	signal(SIGTERM, xpld_shutdownHandler);
	signal(SIGINT,  xpld_shutdownHandler);

	_log(LOG_NOTICE, "Startup");

	/** Main Loop  **/
	for (;;) {
	/* Let XPL run for a while, returning after it hasn't seen any */
	/* activity in 100ms or so                                     */
		xPL_processMessages(100);
	}
}

