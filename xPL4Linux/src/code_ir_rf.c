#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "log.h"
#include "code_ir_rf.h"

void dump_vars(VARIABLES *vars)
{
	VARIABLE	*var;

	for (var = vars->entries; var < vars->entries + vars->count; var++)
		if (var->map)
			_log(LOG_INFO, "    %s:%d:%d:%s", var->name, var->start, var->len, var->map->name);
		else
			_log(LOG_INFO, "    %s:%d:%d", var->name, var->start, var->len);
}

void dump_maps(MAPS *maps)
{
	MAP		*map;
	MAP_ENTRY	*entry;

	for (map = maps->entries; map < maps->entries + maps->count; map++) {
		_log(LOG_INFO, "Map [%s]", map->name);
		for (entry = map->entries; entry < map->entries + map->count; entry++)
			_log(LOG_INFO, "%10.10s 0x%08x", entry->friendly, entry->value);
		_log(LOG_INFO, "");
	}
}

void dump_vals(char *buffer, uint16_t *vals)
{
	int len = 0;

	*buffer = 0;
	while (vals && *vals)
		len += sprintf(buffer + len, len == 0 ? "%d" : ":%d", *(vals++));
}

void dump_codes(CODEDEFS *codes)
{
	CODEDEF	*code;
	char	buffer[256];

	for (code = codes->entries; code < codes->entries + codes->count; code++) {
		_log(LOG_INFO, "%s Code [%s]",
			code->code_type == CODE_TYPE_IR ? "IR" : "RF",
			code->name);
		_log(LOG_INFO, "  bits:   %d", code->bits);
		if (code->init_len) {
			int i, p;

			for (i = 0, p = 0; i < code->init_len; i++)
				p += snprintf(buffer + p, sizeof(buffer) - p, "%02x", code->init[i]);
			_log(LOG_INFO, "  init:   %s", buffer);
		}
		if (code->header) {
			dump_vals(buffer, code->header);
			_log(LOG_INFO, "  header: %s", buffer);
		}
		dump_vals(buffer, code->zero);
		_log(LOG_INFO, "  zero:   %s", buffer);
		dump_vals(buffer, code->one);
		_log(LOG_INFO, "  one:    %s", buffer);
		dump_vals(buffer, code->trail);
		_log(LOG_INFO, "  trail:  %s", buffer);
		if (code->raw_frame) {
			dump_vals(buffer, code->raw_frame);
			_log(LOG_INFO, "  raw:    bit %d, %s", code->raw_frame_bit, buffer);
		}
		if (code->crc) {
			_log(LOG_INFO, "  crc:    %s, start %d, bits %d, pos %d",
				code->crc->name, code->crc_start, code->crc_len, code->crc_pos);
		}
		_log(LOG_INFO, "  repeat: %d", code->repeat);
		dump_vars(&(code->vars));
		_log(LOG_INFO, "");
	}
}

size_t ir_rf_generate_pulses(const CODEDEF* code, uint8_t *cmd, uint8_t cmd_len, uint8_t* buffer, size_t buflen, callback_write_pulse func_write_pulse, void *ctx)
{
	size_t		pos;
	const uint16_t	*data;
	uint8_t		bit, b, mask, p;

	pos = 0;
	for (data = code->header; data && *data; data++)
		pos += func_write_pulse(ctx, buffer + pos, buflen - pos, *data);

	for (p = 0, bit = 0, mask = 0; bit < code->bits; bit++, mask >>= 1) {
		if (!mask) {
			mask = 0x80;
			b = cmd[p++];
		}

		if (code->raw_frame && bit == code->raw_frame_bit)
			for (data = code->raw_frame; *data; data++)
				pos += func_write_pulse(ctx, buffer + pos, buflen - pos, *data);
		else
			for (data = b & mask ? code->one: code->zero; data && *data; data++)
				pos += func_write_pulse(ctx, buffer + pos, buflen - pos, *data);
	}

	if (code->trail)
		for (data = code->trail; data && *data; data++)
			pos += func_write_pulse(ctx, buffer + pos, buflen - pos, *data);

	return pos;
}

bool ir_rf_cmd_set_val(uint8_t* cmd, uint8_t cmd_len, uint64_t val, uint16_t start, uint16_t len, bool reverse)
{
	uint16_t	pos, bit, used;

	if (reverse) {
		uint8_t	mask = 1 << (7 - (start & 7));

		pos = start >> 3;
		if (pos >= cmd_len)
			return false;

		while (len && val) {
			if (val & 1)
				cmd[pos] |= mask;

			val  >>= 1;
			mask >>= 1;
			if (!mask) {
				mask = 0x80;
				pos++;
				if (pos >= cmd_len)
					return false;
			}

			--len;
		}

		return true;
	}

	bit = start + len - 1;
	pos = bit >> 3;
	pos++;
	bit &= 7;
	bit = 7 - bit;

	if (pos > cmd_len)
		return false;

	while (len && pos && val) {
		--pos;
		cmd[pos] |= (uint8_t)(val << bit);
		used = 8 - bit;
		val >>= used;
		len -= used;
		bit = 0;
	}

	return true;
}

bool ir_rf_cmd_from_vars(const CODEDEF *code, callback_get_var func_get_var, void* ctx, uint8_t* outcmd, uint8_t outlen)
{
	VARIABLE	*var;
	MAP_ENTRY	*entry;
	uint64_t	val;
	char*		val_str;
	uint8_t		crc_len, i;
	uint32_t	crc;

	memset(outcmd, 0, outlen);
	for (i = 0; i < code->init_len; i++)
		outcmd[i] = code->init[i];

	for (var = code->vars.entries; var < code->vars.entries + code->vars.count; var++) {
		val_str = func_get_var(ctx, var->name);
		if (!val_str)
			return false;


		val = strtoull(val_str, 0, 16);
		if (var->map)
			for (entry = var->map->entries; entry < var->map->entries +var->map->count; entry++)
				if (0 == strcmp(entry->friendly, val_str))
					val = entry->value;

		if (!ir_rf_cmd_set_val(outcmd, outlen, val, var->start, var->len, var->reverse))
			return false;
	}

	if (!code->crc)
		return true;

	crc = code->crc->func(outcmd, code->crc_start, code->crc_len, &crc_len);
	return ir_rf_cmd_set_val(outcmd, outlen, crc, code->crc_pos, crc_len, false);
}

const char c_reverse[] = "reverse";

char* add_variable(VARIABLES *vars, char* arg, MAPS *maps)
{
	VARIABLE	*var;
	size_t		len;

	if (vars->count == vars->size) {
		VARIABLE	*arr = vars->entries;
		int		new_size = vars->size + 8;

		vars->entries = malloc(sizeof(VARIABLE) * new_size);
		if (arr) {
			memcpy(vars->entries, arr, sizeof(VARIABLE) * vars->size);
			free(arr);
		}
		vars->size = new_size;
	}

	var = vars->entries + vars->count;
	memset(var, 0, sizeof(VARIABLE));

	for (len = 0; arg[len] && arg[len] != ':'; len++);
	var->name = strndup(arg, len);
	arg += len;
	if (*arg != ':')
		goto cleanup;

	var->start = strtol(arg + 1, &arg, 0);
	if (*arg != ':')
		goto cleanup;
	var->len = strtol(arg + 1, &arg, 0);

	if (*arg == ':') {
		int	map;

		arg++;
		for (len = 0; arg[len] && arg[len] != ':' && arg[len] != ','; len++);

		if (0 == strncmp(c_reverse, arg, len))
			var->reverse = true;
		else {
			for (map = 0; map < maps->count && !var->map; map++)
				if (0 == strncmp(maps->entries[map].name, arg, len))
					var->map = maps->entries + map;

			if (!var->map)
				goto cleanup;
		}
		arg += len;
	}

	if (*arg == ':' && var->reverse == false) {
		arg++;
		for (len = 0; arg[len] && arg[len] != ','; len++);

		if (0 == strncmp(c_reverse, arg, len))
			var->reverse = true;
		else
			goto cleanup;

		arg += len;
	}

	vars->count++;
	return arg;

cleanup:
	return arg;
}

char *add_raw(CODEDEF *code, char* arg)
{
	int 	pulses;
	char*	start;

	/* Determine length */
	code->raw_frame_bit = strtoll(arg, &arg, 0);
	for (start = arg, pulses = 0; *start && *start != ','; start++)
		if (*start == ':')
			pulses++;

	code->raw_frame = malloc(sizeof(*(code->raw_frame)) * (pulses + 1));
	for (pulses = 0; *arg == ':'; pulses++)
		code->raw_frame[pulses] = strtoull(arg + 1, &arg, 0);

	return start;
}

char *set_crc(CODEDEF *code, char* arg)
{
	size_t	len;

	for (len = 0; arg[len] && arg[len] != ':'; len++);
	if (arg[len] != ':')
		return arg;

	code->crc = find_crc_func(arg, len);
	if (!code->crc)
		return arg;

	code->crc_start = strtol(arg + len + 1, &arg, 0);
	if (*arg != ':')
		return arg;

	code->crc_len = strtol(arg + 1, &arg, 0);
	if (*arg != ':')
		return arg;

	code->crc_pos = strtol(arg + 1, &arg, 0);
	return arg;
}

bool add_code(char** args, void* data)
{
	CODEDEFS	*codes = data;
	CODEDEF		*code;
	char		*arg;
	size_t		len;

	if (codes->count == codes->size) {
		CODEDEF	*arr = codes->entries;
		int	new_size = codes->size + 8;

		codes->entries = malloc(sizeof(CODEDEF) * new_size);
		if (arr) {
			memcpy(codes->entries, arr, sizeof(CODEDEF) * codes->size);
			free(arr);
		}
		codes->size = new_size;
	}

	code = codes->entries + codes->count;
	memset(code, 0, sizeof(CODEDEF));

	arg = *args;
	for (len = 0; arg[len] && arg[len] != ','; len++);
	code->name = strndup(arg, len);
	arg += len;

	while (*arg) {
		int 		pulses;
		char		*start;
		uint16_t	**val, mult, l;

		arg++;
		for (len = 0; arg[len] && arg[len] != '='; len++);
		if ('=' != arg[len])
			goto cleanup;

		if (0 == strncmp("var", arg, len)) {
			arg = add_variable(&(code->vars), arg + len + 1, codes->maps);
			continue;
		}

		if (0 == strncmp("raw", arg, len)) {
			arg = add_raw(code, arg + len + 1);
			continue;
		}

		if (0 == strncmp("crc", arg, len)) {
			arg = set_crc(code, arg + len + 1);
			continue;
		}

		if (0 == strncmp("type", arg, len)) {
			arg += len + 1;
			for (len = 0; arg[len] && arg[len] != ','; len++);
			if (0 == strncmp("ir", arg, len))
				code->code_type = CODE_TYPE_IR;
			else if (0 == strncmp("rf", arg, len))
				code->code_type = CODE_TYPE_RF;
			else
				goto cleanup;

			arg += len;
			continue;
		}

		if (0 == strncmp("init", arg, len)) {
			int		i;
			uint64_t	v;
			char*		p;

			arg += len + 1;
			p = arg;
			v = strtoull(arg, &arg, 16);
			code->init_len = (arg - p + 1) >> 1;

			i = code->init_len;
			while (i) {
				--i;
				code->init[i] = v & 0xff;
				v >>= 8;
			}
			continue;
		}

		if (0 == strncmp("bits", arg, len)) {
			code->bits = strtol(arg + len + 1, &arg, 0);
			continue;
		}

		if (0 == strncmp("repeat", arg, len)) {
			code->repeat = strtol(arg + len + 1, &arg, 0);
			continue;
		}

		if (0 == strncmp("header", arg, len))
			val = &(code->header);
		else if (0 == strncmp("trail", arg, len))
			val = &(code->trail);
		else if (0 == strncmp("zero", arg, len))
			val = &(code->zero);
		else if (0 == strncmp("one", arg, len))
			val = &(code->one);
		else
			goto cleanup;

		arg += len;

		/* Determine length */
		pulses = 0;
		start = arg;
		while (*start == ':' || *start == '=') {
			mult = 1;
			l = strtoul(start + 1, &start, 0);

			if (*start == '*') {
				mult = l;
				l = strtoul(start + 1, &start, 0);
			}

			pulses += mult;
		}

		*val = malloc(sizeof(uint16_t) * (pulses + 1));

		pulses = 0;
		while (*arg == ':' || *arg == '=') {
			mult = 1;
			l = strtoul(arg + 1, &arg, 0);

			if (*arg == '*') {
				mult = l;
				l = strtoul(arg + 1, &arg, 0);
			}

			while (mult) {
				(*val)[pulses++] = l;
				--mult;
			}
		}

		(*val)[pulses] = 0;
	}

	codes->count++;
	return true;

cleanup:
	_log(LOG_ERR, err_param, *args, arg);
	return false;
}

bool add_map(char** args, void* data)
{
	MAPS		*maps = data;
	MAP		*map;
	MAP_ENTRY	*entry;
	char		*arg;
	size_t		len;

	if (maps->count == maps->size) {
		MAP	*arr = maps->entries;
		int	new_size = maps->size + 8;

		maps->entries = malloc(sizeof(MAP) * new_size);
		if (arr) {
			memcpy(maps->entries, arr, sizeof(MAP) * maps->size);
			free(arr);
		}
		maps->size = new_size;
	}

	map = maps->entries + maps->count;
	memset(map, 0, sizeof(MAP));

	arg = *args;
	for (len = 0; arg[len] && arg[len] != ','; len++);
	map->name = strndup(arg, len);
	arg += len;

	while (*arg) {
		if (map->count == map->size) {
			MAP_ENTRY	*arr = map->entries;
			int		new_size = map->size + 8;

			map->entries = malloc(sizeof(MAP_ENTRY) * new_size);
			if (arr) {
				memcpy(map->entries, arr, sizeof(MAP_ENTRY) * map->size);
				free(arr);
			}
			map->size = new_size;
		}

		entry = map->entries + map->count;
		memset(entry, 0, sizeof(MAP_ENTRY));

		arg++;
		for (len = 0; arg[len] && arg[len] != '='; len++);
		if (!arg[len])
			goto cleanup;

		entry->friendly = strndup(arg, len);
		entry->value = strtol(arg + len + 1, &arg, 0);
		map->count++;
	}

	maps->count++;
	return true;

cleanup:
	_log(LOG_ERR, err_param, *args, arg);
	return false;
}
