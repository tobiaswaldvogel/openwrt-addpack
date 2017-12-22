#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "crc.h"

#define CRC_POLY_KERMIT		0x8408
#define	CRC_START_KERMIT	0x0000

static bool	crc_tab_init = false;
static uint16_t	crc_tab[256];

const CRC_FUNC crc_alg[] = {
	{ "kermit", crc_kermit },
	{ "invert", crc_invert }
};

const CRC_FUNC* find_crc_func(const char* name, size_t len)
{
	const CRC_FUNC	*p;

	for (p = crc_alg; p < crc_alg + (sizeof(crc_alg)/sizeof(crc_alg[0])); p++)
		if (0 == strncmp(name, p->name, len))
			return p;

	return 0;
}

void init_crc_tab()
{
	uint16_t i;
	uint16_t j;
	uint16_t crc;
	uint16_t c;

	for (i=0; i<256; i++) {
		crc = 0;
		c   = i;

		for (j=0; j<8; j++) {
			if ((crc ^ c) & 0x0001)
				crc = ( crc >> 1 ) ^ CRC_POLY_KERMIT;
			else
				crc =   crc >> 1;

			c = c >> 1;
		}

		crc_tab[i] = crc;
	}

	crc_tab_init = true;
}

uint32_t crc_kermit(const uint8_t *data, uint8_t start, uint8_t len, uint8_t* crc_len)
{
	uint16_t	crc;
	uint16_t	low_byte;
	uint16_t	high_byte;
	size_t		i;

	if (! crc_tab_init)
		init_crc_tab();

	crc = CRC_START_KERMIT;

	if (data != NULL)
		for (i = 0; i < (len >> 3); i++) {
			crc = (crc >> 8) ^ crc_tab[ (crc ^ (uint16_t) data[i]) & 0x00FF ];
		}

//		for (i = 0; i < len; i++) {
//			crc = (crc >> 8) ^ crc_tab[ (crc ^ (uint16_t) data[i]) & 0x00FF ];
//		}

		low_byte  = (crc & 0xff00) >> 8;
	high_byte = (crc & 0x00ff) << 8;
	crc       = low_byte | high_byte;

	*crc_len = 16;
	return crc;
}


uint32_t crc_invert(const uint8_t *data, uint8_t start, uint8_t len, uint8_t* crc_len)
{
	uint32_t	crc = 0;
	uint16_t	pos;
	uint8_t		mask, val;

	*crc_len = len;

	mask = 1 << (7 - (start & 7));
	pos = start >> 3;
	val = data[pos];

	while (len) {
		crc <<= 1;
		if (val & mask)
			crc |= 1;

		mask >>= 1;
		if (!mask) {
			mask = 0x80;
			pos++;
			val = data[pos];
		}
		--len;
	}

	return ~crc;
}
