typedef struct {
	char*		name;
	char*		value;
	int			multiplier;
	int 			divisor;
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
	timer_t	 timer_id;
	int		repeat_count;
	xPL_Message	*msg;
} SCHEDULED_MSG;

extern FORWARDS	fwds;

void dump_fwds();
bool add_fwd(char** args, void *unused);
void fwd_timerHandler(int sig, siginfo_t *si, void *uc);
void fwd_msg_handler(xPL_Service *service, xPL_Message *msg, xPL_ObjectPtr data);