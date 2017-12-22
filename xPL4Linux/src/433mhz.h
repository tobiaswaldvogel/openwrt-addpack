#include <stdint.h>

#define array(x) (sizeof(x)/sizeof(x[0])), x

typedef struct CODE_MAP {
	char		*friendly;
	uint32_t	value;
} CODE_MAP;

typedef struct CODE_PARAM {
	char			*name;
	uint8_t			start;
	uint8_t			len;
	const CODE_MAP		*map;
} CODE_PARAM;

typedef struct CODE_FRAME {
	uint32_t	code;
	uint32_t	frame;
} CODE_FRAME;

typedef struct CODE_FRAMEDEF {
	uint8_t			code_len;
	uint8_t			frame_len;
	uint8_t			frames_count;
	const CODE_FRAME	*frames;
	uint8_t			start;
} CODE_FRAMEDEF;

typedef struct CODE433 {
	char			*name;
	uint8_t			cmd_len;
	uint8_t			framedef_count;
	const CODE_FRAMEDEF	*framedef;
	uint32_t		init;
	uint16_t		trail_len;
	uint16_t		short_len;
	uint16_t		long_len;
	uint16_t		head_len;
	const CODE_PARAM	param[4];
} CODE433;

static const CODE_MAP map_it_cmd[] = {
	{ "off", 0 },
	{ "on",  1 },
	{ "dim", 2 },
	{ }
};

static const CODE_MAP map_rsl366_cmd[] = {
	{ "off", 4 },
	{ "on",  0 },
	{ }
};

static const CODE_MAP map_screen_cmd[] = {
	{ "up",   2 },
	{ "down", 4 },
	{ "stop", 8 },
	{ }
};

static const CODE_MAP map_rsl2_button[] = {
	{ "1", 0b10 },
	{ "2", 0b01 },
	{ "3", 0b11 },
	{ "4", 0b00 },
	{ }
};

static const CODE_FRAME frames_it[] = {
	{ 0, 0b0001 },
	{ 1, 0b0100 },
};

static const CODE_FRAME frames_it_dim[] = {
	{ 0, 0b0000 },
};

static const CODE_FRAMEDEF framedef_it[] = {
	{ 1, 4, array(frames_it) }
};

static const CODE_FRAMEDEF framedef_it_dim[] = { 
	{ 1, 4, array(frames_it),     28 },
	{ 1, 4, array(frames_it_dim), 27 },
	{ 1, 4, array(frames_it) ,    0  },
};

static const CODE_FRAME frames_rsl366[] = {
	{ 0, 0x6666, },
	{ 1, 0x5666, },
	{ 2, 0x6566, },
	{ 3, 0x6656, },
	{ 4, 0x6665, },
};

static const CODE_FRAMEDEF framedef_rsl366[] = {
	{ 4, 16, array(frames_rsl366) }
};

static const CODE_FRAME frames_2bit[] = {
	{ 0, 0b01, },
	{ 1, 0b10, },
};

static const CODE_FRAMEDEF framedef_2bit[] = {
	{ 1, 2, array(frames_2bit) }
};

static const CODE433 rf433_codes[] = {
	{
		"it", 36, array(framedef_it_dim), 0, 11450, 275, 1275, 2675, {
			{ "id", 0, 26 },
			{ "button", 28, 4 },
			{ "dim", 32, 4 },
		}
	}, {
		"it", 32, array(framedef_it), 0, 11450, 275, 1275, 2675, {
			{ "id", 0, 26 },
			{ "button", 28, 4 },
			{ "cmd", 27,1, map_it_cmd }
		}
	}, {
		"rsl366", 3, array(framedef_rsl366), 0, 13500, 430, 1275, 0, {
			{ "id", 0, 2 },
			{ "cmd", 2, 1, map_rsl366_cmd }
		}
	} ,{
		"rsl2", 32, array(framedef_2bit), 1<<31, 7000, 580, 1275, 0, {
			{ "id", 6,26 },
			{ "button", 2, 2, map_rsl2_button },
			{ "cmd", 4, 2 }
		}
		
	},{
		"screen", 24, array(framedef_2bit), 0, 6600, 220, 650, 0, {
			{ "id", 0,20 },
			{ "cmd", 20, 4, map_screen_cmd }
		}
	}
};

#define rf433_codes_count (sizeof(rf433_codes) / sizeof(rf433_codes[0]))