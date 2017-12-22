#include "crc.h"

enum CODE_TYPE { CODE_TYPE_IR, CODE_TYPE_RF };

typedef struct {
	uint32_t	value;
	char*		friendly;
} MAP_ENTRY;

typedef struct {
	char		*name;
	MAP_ENTRY	*entries;
	int		count;
	int		size;
} MAP;

typedef struct {
	MAP	*entries;
	int	count;
	int	size;
} MAPS;

typedef struct {
	char	*name;
	uint8_t	start;
	uint8_t	len;
	MAP	*map;
	bool	reverse;
} VARIABLE;

typedef struct {
	VARIABLE	*entries;
	int		count;
	int		size;
} VARIABLES;

typedef struct {
	char		*name;
	enum CODE_TYPE	code_type;
	uint8_t		bits;
	uint8_t		repeat;
	uint16_t	*header;
	uint16_t	*one;
	uint16_t	*zero;
	uint16_t	*trail;
	uint8_t		init[8];
	uint8_t		init_len;
	VARIABLES	vars;
	const CRC_FUNC	*crc;
	uint8_t		crc_start;
	uint8_t		crc_len;
	uint8_t		crc_pos;
	uint8_t		raw_frame_bit;
	uint16_t	*raw_frame;
} CODEDEF;

typedef struct {
	MAPS	*maps;
	CODEDEF	*entries;
	int	count;
	int	size;
} CODEDEFS;

typedef char* (*callback_get_var)(void *ctx, char *name);
typedef size_t (*callback_write_pulse)(void *ctx, uint8_t* buffer, size_t buflen, uint16_t time_ms);

void dump_vars(VARIABLES *vars);
void dump_maps(MAPS *maps);
void dump_vals(char *buffer, uint16_t *vals);
void dump_codes();
bool ir_rf_cmd_from_vars(const CODEDEF *code, callback_get_var func_get_var, void* ctx, uint8_t* outcmd, uint8_t outlen);
size_t ir_rf_generate_pulses(const CODEDEF* code, uint8_t *cmd, uint8_t cmd_len, uint8_t* buffer, size_t buflen, callback_write_pulse func_write_pulse, void *ctx);
bool add_code(char** args, void *codes);
bool add_map(char** args, void* maps);
