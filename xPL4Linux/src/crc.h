#include <stdint.h>

typedef uint32_t (*crc_func)(const uint8_t *data, uint8_t start, uint8_t len, uint8_t* crc_len);

typedef struct {
	char*		name;
	crc_func	func;
} CRC_FUNC;

const CRC_FUNC* find_crc_func(const char* name, size_t len);
uint32_t crc_kermit(const uint8_t *data, uint8_t start, uint8_t len, uint8_t* crc_len);
uint32_t crc_invert(const uint8_t *data, uint8_t start, uint8_t len, uint8_t* crc_len);