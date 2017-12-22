#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "433mhz.h"
#include "log.h"
#include "alsa.h"

#define VERSION "1.0"
#define MAX_PULSE 255

/*

typedef struct CODE {
	const CODE433		*codedef;
	uint8_t			param_count;
	uint32_t		param[4];
} CODE;

static const char pcm_device[] = "default";


extern uint32_t round_div(uint32_t d, uint32_t n);


__inline bool translate_frame_in(const CODE_FRAMEDEF *framedef, uint32_t *frame)
{
	const CODE_FRAME	*frame_table;

	for (frame_table = framedef->frames; frame_table < framedef->frames + framedef->frames_count; frame_table++)
		if (*frame == frame_table->frame) {
			*frame = frame_table->code;
			return true;
		}

	return false;
}


__inline bool get_next_sample(uint8_t *data, size_t *pos, uint8_t channels, uint8_t bps)
{
	int16_t	sample;

	if (bps == 8) {
		sample = data[(*pos)++];

		if (channels == 2) {
			sample += data[(*pos)++];
			sample >>= 1;
		}
		return sample > 1<<6;
	}
	
	sample =  data[(*pos)++];
	sample += data[(*pos)++] << 8;

	if (channels == 2) {
		int32_t avg = sample;

		sample = data[(*pos)++];
		sample += data[(*pos)++] << 8;
		avg += sample;

		sample = avg >> 1;
	}

	return sample > -(1<<14);
}

__inline bool get_prev_sample(uint8_t *data, size_t *pos, uint8_t channels, uint8_t bps)
{
	int16_t	sample;

	if (bps == 8) {
		if (!*pos)
			return false;
		sample = data[--(*pos)];

		if (channels == 2 && *pos) {
			sample += data[--(*pos)];
			sample >>= 1;
		}
		return sample > 1<<6;
	}

	if (*pos < 2)
		return false;

	sample  = data[--(*pos)] << 8;
	sample += data[--(*pos)];

	if (channels == 2 && *pos >= 2) {
		int32_t avg = sample;

		sample  = data[--(*pos)] << 8;
		sample += data[--(*pos)];
		avg += sample;

		sample = avg >> 1;
	}
	return sample > -(1<<14);
}

size_t find_code_pulse(uint8_t* data, size_t len, uint32_t rate, uint8_t channels, uint8_t bps, uint16_t* pulse, uint8_t *pulse_count)
{
	uint8_t		pulse_max;
	size_t		pos, cmd_start, cmd_end;

	bool		last_sample;
	size_t		pulse_len;
	uint32_t	pulse_len_t;

	size_t		bytes_per_sample = (bps == 8 ? 1 : 2) * channels;
	size_t		min_footer_len = rate>>8;	// approx. 4ms
	int32_t		skip = min_footer_len * bytes_per_sample;

	uint32_t	noise_limit = rate>>14, noise_limit_total = rate>>11;
	size_t		noise, noise_len, noise_len_total;

	pulse_max = *pulse_count;
	*pulse_count = 0;

	for (pos = 0; pos < len; pos += skip) {
		if (!get_next_sample(data, &pos, channels, bps))
			continue;

		pulse_len = 0;
		noise_len_total = 0;

		// Find end of pulse 
		cmd_end = pos;
		while (cmd_end < len) {
			while (cmd_end < len && get_next_sample(data, &cmd_end, channels, bps))
				pulse_len++;

			noise = cmd_end;
			noise_len = 0;
			while (noise < len && !get_next_sample(data, &noise, channels, bps))
				noise_len++;

			if (noise_len > noise_limit)
				break;

			noise_len_total += noise_len;
			if (noise_len_total > noise_limit_total)
				break;

			cmd_end = noise;
			pulse_len += noise_len;
		}

		// Find start of pulse
		cmd_start = pos;
		while (cmd_start > 0) {
			while (cmd_start > 0 && get_prev_sample(data, &cmd_start, channels, bps))
				pulse_len++;

			noise = cmd_start;
			noise_len = 0;
			while (noise > 0 && !get_prev_sample(data, &noise, channels, bps))
				noise_len++;

			if (noise_len > noise_limit)
				break;

			noise_len_total += noise_len;
			if (noise_len_total > noise_limit_total)
				break;

			cmd_start = noise;
			pulse_len += noise_len;
		}


		if (pulse_len < min_footer_len)
			continue;

		break;
	}

	if (pos >= len)
		return len;

	pos = cmd_start;

	pulse_len_t = pulse_len;
	pulse_len_t *= 1000000;
	pulse_len_t /= rate;

	pulse[(*pulse_count)++] = pulse_len_t;
	last_sample = false;
	pulse_len = 1;

	while (pos > 0 && *pulse_count < pulse_max) {
		if (last_sample == get_prev_sample(data, &pos, channels, bps)) {
			pulse_len++;

			if (pos) // Store the first one  if pos = 0
				continue;
		}

		last_sample = !last_sample;

		pulse_len_t = pulse_len;
		pulse_len_t *= 1000000;
		pulse_len_t /= rate;
		pulse[(*pulse_count)++] = (uint16_t)pulse_len_t;

		pulse_len = 1;
	}

	return cmd_end;
}

bool parse_code(const uint16_t *pulse, const uint8_t pulse_count, const CODE433 *codedef, CODE *code)
{
	uint16_t	short_min, short_max, long_min, long_max;

	uint8_t		frame, bits, bits_total, p;
	uint16_t	pulse_len, pulse_len_test;
	uint32_t	c, b;

	int32_t		delta;
	uint16_t	tolerance;

	uint16_t		noise_limit = 70;
	const CODE_FRAMEDEF	*framedef;

	delta = pulse[0] - codedef->trail_len;
	if (delta < 0)
		delta = -delta;
	if (delta > (codedef->trail_len)>>3)
		return false;

_log(LOG_ERR, "trying to parse %s", codedef->name);


	tolerance = (codedef->short_len) >> 1;
	short_min = codedef->short_len - tolerance;
	short_max = codedef->short_len + tolerance;
	long_min = codedef->long_len - tolerance;
	long_max = codedef->long_len + tolerance;

	frame = codedef->cmd_len;
	if (codedef->head_len)
		frame++;

	framedef = codedef->framedef;
	bits_total = 0;

	for (c=0, p=2; frame > 0 && p < pulse_count; --frame) {
		if (frame == 1 && codedef->head_len) {
			long_min = codedef->head_len - tolerance;
			long_max = codedef->head_len + tolerance;
			bits = 2;
		} else {
			if (frame <= framedef->start)
				framedef++;
			bits = framedef->frame_len;
		}

		for (b = 0; bits > 0 && p < pulse_count; --bits) {
			b >>= 1;
			pulse_len = pulse[p++];

			// Consume pulse until at least short_min
			while (pulse_len < short_min && p < pulse_count) {
				pulse_len += pulse[p++];
				pulse_len += pulse[p++];
			}

			if (pulse_len <= short_max) {
				// Check if next is noise because this might be part of a long pulse
				if (pulse[p] > noise_limit)
					continue;

				pulse_len_test = pulse_len + pulse[p] + pulse[p + 1];
				if (pulse_len_test < long_min || pulse_len_test > long_max)
					continue;
			}

			while (pulse_len < long_min && p > 1) {
				pulse_len += pulse[p++];
				pulse_len += pulse[p++];
			}

			if (pulse_len > long_max)
				return false;
			b |= 1<<31;
		}

		// Evaluate frame
		if (frame == 1 && codedef->head_len) {
			if (b != 1<<30)			// Header is long-short
				return false;
		} else {
			b >>= 32 - framedef->frame_len;

			if (!translate_frame_in(framedef, &b)) {
_log(LOG_INFO, "Wrong frame %x at %d", b, frame);
				return false;
			}

			c >>= framedef->code_len;
			c |= b<<(32 - framedef->code_len);
			bits_total += framedef->code_len;
		}
	}

	if (frame) {
_log(LOG_INFO, "Not enough pulses");
		return false;
	}

	c >>= 32 - bits_total;

	code->param_count = 0;
	for (p = 0; p < 4; p++) {
		uint32_t			mask;
		uint8_t				shift, mask_len;
		const CODE_PARAM	*prm = codedef->param + p;
		
		if (prm->len == 0) {
			code->param[p] = 0;
			continue;
		}

		shift = 0;
		framedef = codedef->framedef;
		for (frame = codedef->cmd_len; frame > prm->start + prm->len; --frame) {
			if (frame <= framedef->start)
				framedef++;
			shift += framedef->code_len;
		}

		mask_len = 0;
		for (; frame > prm->start; --frame) {
			if (frame <= framedef->start)
				framedef++;
			mask_len += framedef->code_len;
		}

		mask = prm->len == 32 ? -1 : (1 << mask_len) - 1;
		code->param[code->param_count++] = (c >> shift) & mask;

		// Remove these bits from the code
		mask <<= shift;
		c &= ~mask;
	}

	if (c != codedef->init)
		return false;

_log(LOG_ERR, "!!success!! %s", codedef->name);
	code->codedef = codedef;
	return true;
}

snd_pcm_t		*capture_dev = 0; 


void shutdownHandler(int onSignal) {
	if (capture_dev) {
		snd_pcm_drain(capture_dev);
		snd_pcm_close(capture_dev);
	}

	exit(0);
}

//int capture(const char* card)
void process_samples(unsigned int rate, unsigned int channels, unsigned int bps)
{
	unsigned int		frame_size = channels * (bps == 16 ? 2 : 1);
	snd_pcm_uframes_t	frames, buf_size = (rate>>2) * frame_size;
	unsigned char		buffer[buf_size];
	unsigned int		buf_len, pos;

	CODE		code;
	CODE		codes[64];
	uint8_t		i, j, matches_max, pulse_count, code_count, percent;

	uint16_t	pulse[MAX_PULSE];
	size_t		codeset;

	char		cmd[256];
	size_t		cmd_pos;

_log(LOG_INFO, "Effective rate %d", rate);

	for (pos = 0, buf_len = 0, code_count = 0;;) {
		// Fill buffer 
		if ((buf_len - pos) < (buf_size>>2)) {
			if (pos != buf_len) {
				memmove(buffer, buffer + pos, buf_len - pos);
				buf_len -= pos;
			}
			else {
			
				buf_len = 0;
			}

			frames = snd_pcm_readi(capture_dev, buffer + buf_len, (buf_size - buf_len) / frame_size);
			if ((long)frames < 0) {
				snd_pcm_state_t status;
				status = snd_pcm_state(capture_dev);
				fprintf(stderr, "pcm read error %s  %d\n", snd_strerror(frames), status);
				break;
			}

			buf_len += frames * frame_size;
			pos = 0;
		}

		pulse_count = (uint8_t)(sizeof(pulse) / sizeof(pulse[0]));
		pos += find_code_pulse(buffer + pos, buf_len - pos, rate, channels, bps, pulse, &pulse_count);

		if (pos < buf_len) {
_log(LOG_INFO, "12 pos %d, len %d, pulse_count %d, code_count %d", pos, buf_len, pulse_count, code_count);
			codes[code_count].codedef = 0;
			for (codeset = 0; codeset <rf433_codes_count; codeset++)
				if (parse_code(pulse, pulse_count, rf433_codes + codeset, codes + code_count)) {
					break;
				}

			code_count++;
			if (code_count < sizeof(codes) / sizeof(codes[0]))
				continue;
		}

		if (code_count < 3) {
			code_count = 0;
			continue;
		}

		for (matches_max = 0, i = 0; i < code_count; i++) {
			uint8_t	c, matches = 0;

			if (!codes[i].codedef)
				continue;

			for (c = 0; c < code_count; c++) {
				if (codes[i].codedef != codes[c].codedef ||
					codes[i].param[0] != codes[c].param[0])
					continue;

				matches++;
			}

			if (matches > matches_max) {
				matches_max = matches;

				code.codedef = codes[i].codedef;
				code.param_count = codes[i].param_count;

				for (j = 0; j < code.param_count; j++)
					code.param[j] = codes[i].param[j];

				percent = matches * 100 / code_count;
			}
		}

		code_count = 0;

		if (!matches_max)
			continue;

		cmd_pos = snprintf(cmd, sizeof(cmd), "%s", code.codedef->name);
		for (i = 0; i < code.param_count; i++) {
			const CODE_MAP	*map = code.codedef->param[i].map;
			uint32_t		value = code.param[i];

			if (map) {
				while (value != map->value && (map->value || map->friendly))
					map++;

				if (map->value == 0 && map->friendly == 0)
					map = 0;
			}

			if (map)
				cmd_pos += snprintf(cmd+cmd_pos, sizeof(cmd)-cmd_pos, ",%s=%s", code.codedef->param[i].name, map->friendly);
			else
				cmd_pos += snprintf(cmd+cmd_pos, sizeof(cmd)-cmd_pos, ",%s=%X", code.codedef->param[i].name, value);
		}

		snprintf(cmd+cmd_pos, sizeof(cmd)-cmd_pos, "   (%d %%)", percent);
		_log(LOG_INFO, "%s", cmd);
	}
}
*/

int test_main(int argc, char* argv[]) {
/*	unsigned int	rate = 44000, channels = 1, bps = 16;

	signal(SIGTERM, shutdownHandler);
	signal(SIGINT, shutdownHandler);

	alsa_max_volume(pcm_device);
	if (alsa_init_capture(pcm_device, &rate, channels, bps, &capture_dev))
		return -1;

	process_samples(rate, channels, bps);
*/
	return 0;
}

