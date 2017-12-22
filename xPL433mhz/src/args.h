#include <syslog.h>

typedef bool (*PARAM_FUNC)(char **);

typedef struct {
	char		*name_short;
	char		*name_long;
	char		*help;
	int		args;
	PARAM_FUNC	func;
} PARAM;

void _log(int priority, const char* format, ...);
bool parse_args(int argc, char *argv[], const PARAM *params, int count);