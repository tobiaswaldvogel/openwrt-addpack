#include <syslog.h>

extern	int	loglevel;
extern	int	usesyslog;

void _log(int priority, const char* format, ...);
