typedef struct {
	uint16_t	id;
	const char	*name;
} DEV_NAME;

typedef struct {
	struct sockaddr_in	addr;
	uint8_t			mac[6];
	uint16_t			devtype;
	char			devtype_name[16];
	char			dev_name[16];
	uint16_t			count;
	uint8_t			key[16];
	uint8_t			id[4];
	sem_t			sem_dev;
	int				temperature;
} BL_DEVICE;

void bl_msg_handler(xPL_MessagePtr msg, xPL_ObjectPtr data);
int bl_startup();
void bl_shutdown();
void bl_io_handler(int signal);