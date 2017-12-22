typedef bool (*PARAM_FUNC)(char **, void *data);

typedef struct {
	const char		*name_short;
	const char		*name_long;
	const char		*help;
	const int		args;
	const PARAM_FUNC	func;
	void			*data;
} PARAM;

typedef struct {
	const PARAM	*params;
	int		count;
	const char	*cmdline;
} PARAM_INFO;

bool parse_args(int argc, char *argv[], const PARAM *params, int count);

static const char err_param[] = "Error in parameter %s, around %s\n";
