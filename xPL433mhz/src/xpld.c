#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include "xPL.h"

#include "args.h"

#define VERSION "1.0"

typedef struct {
	xPL_Service	*service;
	char*		src[3];
	char*		tgt[3];
	char*		name[4];
	char*		value[4];
} ALIAS;

ALIAS		alias_list[256];
uint8_t		alias_count = 0;

void alias_msg_handler(xPL_Service *service, xPL_Message *msg, xPL_ObjectPtr data)
{
	ALIAS			*a = data;
	xPL_Message		*out_msg;
	xPL_NameValueList	*body;
	xPL_NameValuePair	**nv;
	xPL_Service		src;	/* dummy service for setting source */
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

	if ((out_msg = xPL_createTargetedMessage(&src, xPL_getMessageType(msg), a->tgt[0], a->tgt[1], a->tgt[2])) == NULL)
		return;

	pos = sprintf(buffer, "%s-%s.%s -> %s-%s.%s : %s.%s {",
		src.serviceVendor, src.serviceDeviceID, src.serviceInstanceID,
		a->src[0], a->src[1], a->src[2],
		xPL_getSchemaClass(msg), xPL_getSchemaType(msg));

	if (body)
		for (i = 0, nv = body->namedValues; i < body->namedValueCount; i++)
			pos += sprintf(buffer+pos, " %s=%s", nv[i]->itemName, nv[i]->itemValue);

	pos += sprintf(buffer+pos, " } => %s-%s.%s",
		a->src[0], a->tgt[1], a->tgt[2]);

	xPL_setSchema(out_msg, xPL_getSchemaClass(msg), xPL_getSchemaType(msg));

	for (i = 0; i < 4; i++)
		if (a->name[i] && strcmp(a->name[i], "svg")) {
			xPL_addMessageNamedValue(out_msg, a->name[i], a->value[i]);
			pos += sprintf(buffer+pos, ",%s=%s", a->name[i], a->value[i]);
		}

	if (body)
		for (i = 0, nv = body->namedValues; i < body->namedValueCount; i++)
			xPL_setMessageNamedValue(out_msg, nv[i]->itemName, nv[i]->itemValue);

	_log(LOG_INFO, "%s", buffer);

	xPL_sendMessage(out_msg);
	xPL_releaseMessage(out_msg);
}

void start_alias()
{
	int		i;
	ALIAS		*a;
	xPL_ServicePtr	service;

	for (i = 0, a = alias_list; i < alias_count; i++, a++) {
		service =  xPL_createService(a->src[0], a->src[1], a->src[2]);
		xPL_setServiceVersion(service, VERSION);
		xPL_setServiceEnabled(service, TRUE);
		xPL_addServiceListener(service, alias_msg_handler, xPL_MESSAGE_ANY, NULL, NULL, a);
		a->service =  service;
	}
}

void free_alias(ALIAS *a)
{
	int	i;

	if (a->service) {
		xPL_setServiceEnabled(a->service, FALSE);
		xPL_releaseService(a->service);
	}

	for (i = 0; i < 3; i++) {
		if (a->src[i])
			free(a->src[i]);
		if (a->tgt[i])
			free(a->tgt[i]);
	}
	for (i = 0; i < 4; i++) {
		if (a->name[i])
			free(a->name[i]);
		if (a->value[i])
			free(a->value[i]);
	}
}

bool add_alias(char** args)
{
	char	*start, **elem, sep;
	ALIAS	*alias;
	size_t	len;
	int	i, e, p;

	alias = alias_list + alias_count;

	memset(alias, 0, sizeof(*alias));
	start = *args;

	for (i = 0; i < 2; i++) {
		elem = i == 0 ? alias->src : alias->tgt;
		for (e = 0; e < 3; e++) {
			if (e == 0)
				sep = '-';
			else if (e == 1)
				sep = '.';
			else if (i == 0)
				sep = '=';
			else
				sep = ',';

			for (len = 0; start[len] && start[len] != sep; len++);
			if (start[len] == 0)
				if (i != 1 || e != 2)
					goto cleanup;

			elem[e] = strndup(start, len);
			if (elem[e] == 0)
				goto cleanup;

			start += len + 1;
		}
	}

	for (p = 0; start[-1] != 0 && p < 4; p++) {
		for (len = 0; start[len] && start[len] != '='; len++);
		if (start[len] == 0)
			goto cleanup;

		alias->name[p] = strndup(start, len);
		if (alias->name[p] == 0)
			goto cleanup;

		start += len + 1;

		for (len = 0; start[len] && start[len] != ','; len++);

		alias->value[p] = strndup(start, len);
		if (alias->value[p] == 0)
			goto cleanup;

		start += len + 1;
	}

	alias_count++;
	return true;

cleanup:
	free_alias(alias);
	return false;
}

void xpld_shutdownHandler(int onSignal) {
	int	i;

	for (i = 0; i < alias_count; i++)
		free_alias(alias_list + i);

	xPL_shutdown();
	_log(LOG_INFO, "Shutdown");
	exit(0);
}

int xpld_main(int argc, String argv[]) {
	static const PARAM	params[] = {
		{ "a", "alias", "Add alias", 1, add_alias },
	};

	if (!xPL_parseCommonArgs(&argc, argv, FALSE)) {
		_log(LOG_ERR, "Unable to start xPL");
		return -1;
	}

	if (!parse_args(argc, argv, params, sizeof(params)/sizeof(params[0])))
		return -2;

	/* Start xPL up */
	if (!xPL_initialize(xPL_getParsedConnectionType())) {
		_log(LOG_ERR, "Unable to start xPL");
		return -3;
	}

	start_alias();

	/* Install signal traps for proper shutdown */
	signal(SIGTERM, xpld_shutdownHandler);
	signal(SIGINT,  xpld_shutdownHandler);

	_log(LOG_INFO, "Startup");

	/** Main Loop  **/
	for (;;) {
	/* Let XPL run for a while, returning after it hasn't seen any */
	/* activity in 100ms or so                                     */
		xPL_processMessages(100);
	}
}

