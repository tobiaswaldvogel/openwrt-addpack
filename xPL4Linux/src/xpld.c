#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <semaphore.h>

#include "xPL.h"
#include "code_ir_rf.h"
#include "xpld.h"
#include "xplfwd.h"
#include "xplbl.h"
#include "xpl433mhz.h"

#include "args.h"
#include "log.h"

#define VERSION "1.0"

static const char bl_vendor_id[] = "broadlink";
static const char xpl433mhz_vendor[] = "433mhz";

char		iface[IFNAMSIZ] = "br-lan";

FORWARDS	fwds = { 0, 0, 0 };
MAPS		maps = { 0, 0, 0 };
CODEDEFS	codes = { &maps, 0, 0, 0 };

xPL_ServicePtr	bl_xpl_service = 0;
xPL_ServicePtr	xpl433mhz_service = 0;

int			xpl_hub = 1;
int			xpl_log = 0;

bool set_iface(char** args, void *unused)
{
	strncpy(iface, *args, sizeof(iface));
	return true;
}

bool enable_xpl_logger(char** unused, void *data)
{
	*((int*)data) = 1;
	return true;
}

bool disable_xpl_hub(char** unused, void *data)
{
	*((int*)data) = 0;
	return true;
}

void logger_msg_handler(xPL_Message *msg, xPL_ObjectPtr data)
{
	xPL_NameValueList	*body;
	xPL_NameValuePair	**nv;
	int				i;
	char			buffer[256];
	int				pos = 0;

	body                  = xPL_getMessageBody(msg);

	pos = sprintf(buffer, "xPL log %s-%s.%s -> %s-%s.%s : %s.%s {",
		xPL_getSourceVendor(msg), xPL_getSourceDeviceID(msg), xPL_getSourceInstanceID(msg),
		xPL_getTargetVendor(msg), xPL_getTargetDeviceID(msg), xPL_getTargetInstanceID(msg),
		xPL_getSchemaClass(msg), xPL_getSchemaType(msg));
	
	if (body)
		for (i = 0, nv = body->namedValues; i < body->namedValueCount; i++) 
			pos += sprintf(buffer+pos, " %s=%s", nv[i]->itemName, nv[i]->itemValue);

	pos += sprintf(buffer+pos, " }");
	_log(LOG_NOTICE, "%s", buffer);
}

void xpld_shutdownHandler(int onSignal)
{
	FORWARD		*fwd;

	if (xpl_hub)
		xPL_stopHub();
	
	for (fwd = fwds.entries; fwd < fwds.entries + fwds.count; fwd++) {
		if (!fwd->service)
			continue;

		xPL_setServiceEnabled(fwd->service, FALSE);
		xPL_releaseService(fwd->service);
	}
	
	if (bl_xpl_service) {
		xPL_setServiceEnabled(bl_xpl_service, FALSE);
		xPL_releaseService(bl_xpl_service);

		bl_shutdown();
	}	

	if (xpl433mhz_service) {
		xPL_setServiceEnabled(xpl433mhz_service, FALSE);
		xPL_releaseService(xpl433mhz_service);
	}
	
	xPL_shutdown();
	_log(LOG_NOTICE, "Shutdown");
	exit(0);
}

int xpld_main(int argc, String argv[])
{
	FORWARD		*fwd;

	static const PARAM	params[] = {
		{ "f", "fwd", "Add fwd", 1, add_fwd },
		{ "n", "net", "Set network interface", 1, set_iface },
		{ "m", "map", "Value map", 1, add_map, &maps },
		{ "c", "code", "Code definition", 1, add_code, &codes },
		{ "d", "dump", "Dump xPL messages", 0, enable_xpl_logger, &xpl_log },
		{ "x", "nohub", "Disable built-in hub", 0, disable_xpl_hub, &xpl_hub },
	};

	struct sigaction	sa;

	if (!xPL_parseCommonArgs(&argc, argv, FALSE)) {
		_log(LOG_EMERG, "Unable to start xPL");
		return -1;
	}

	if (!parse_args(argc, argv, params, sizeof(params)/sizeof(params[0])))
		return -2;

	if (xPL_initialize(xcStandAlone)) {
		_log(LOG_NOTICE, "Startup");
	} else if (xPL_initialize(xPL_getParsedConnectionType())) {
		_log(LOG_NOTICE, "Startup (using running hub)");
		xpl_hub = 0;
	} else {
		_log(LOG_EMERG, "Unable to start xPL");
		return -3;
	}

	dump_maps(&maps);
	dump_codes(&codes);
	dump_fwds();

	sigemptyset(&sa.sa_mask);
	sa.sa_handler = xpld_shutdownHandler;
	sigaction(SIGTERM, &sa, NULL) ;
	sigaction(SIGINT, &sa, NULL) ;

	sa.sa_handler = bl_io_handler;
	sigaction(SIGIO, &sa, NULL);

	sa.sa_flags = SA_SIGINFO;
	sa.sa_handler = 0;
	sa.sa_sigaction = fwd_timerHandler;
	sigaction(SIGRTMIN, &sa, NULL) ;
	
	if (xpl_log)
		xPL_addMessageListener(logger_msg_handler, NULL);
	
	if (xpl_hub) {
		_log(LOG_NOTICE, "Starting built-in xPL hub");
		xPL_startHub();
	}
	
	if (0 == bl_startup()) {
		bl_xpl_service =  xPL_createService((char*)bl_vendor_id, "default", "default");
		xPL_setServiceVersion(bl_xpl_service, VERSION);
		xPL_setServiceEnabled(bl_xpl_service, TRUE);
		xPL_addMessageListener(bl_msg_handler, (void*)bl_vendor_id);
	}

	if (0 == xpl433mhz_startup()) {
		xpl433mhz_service =  xPL_createService((char*)xpl433mhz_vendor, "sender", "default");
		xPL_setServiceVersion(xpl433mhz_service, VERSION);
		xPL_setServiceEnabled(xpl433mhz_service, TRUE);
		xPL_addMessageListener(xpl433mhz_msg_handler, (void*)xpl433mhz_vendor);
	}
	
	for (fwd = fwds.entries; fwd < fwds.entries + fwds.count; fwd++) {
		fwd->service =  xPL_createService(fwd->id[0], fwd->id[1], fwd->id[2]);
		xPL_setServiceVersion(fwd->service, VERSION);
		xPL_setServiceEnabled(fwd->service, TRUE);
		xPL_addServiceListener(fwd->service, fwd_msg_handler, xPL_MESSAGE_ANY, NULL, NULL, fwd);
	}
	
	xPL_processMessages(-1);
	return 0;
}

