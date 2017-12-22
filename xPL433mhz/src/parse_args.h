typedef bool (*PARAM_FUNC)(char **);

typedef struct {
	char		*name_short;
	char		*name_long;
	char		*help;
	int		args;
	PARAM_FUNC	func;
} PARAM;
