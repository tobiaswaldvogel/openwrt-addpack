#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <alsa/asoundlib.h>
#include "xPL.h"

#include "args.h"
#include "433mhz.h"

#define VERSION "1.0"

/* global/static variables */
static const char pcm_device[] = "default";
static const char rf_code_vendor[] = "433mhz";

xPL_ServicePtr	rf_code_service = 0;
sem_t		sem_sound;

uint32_t round_div(uint32_t d, uint32_t n)
{
	return  (d % n) < (n >> 1) ? d / n : (d / n) + 1;
}


int alsa_max_volume(const char* card)
{
	int rc, retval = 2;
	long min, max, vol;
	snd_mixer_t *handle = 0;
	snd_mixer_elem_t *elem;

	rc = snd_mixer_open(&handle, 0);
	if (rc < 0) {
		_log(LOG_ERR, "Can't open mixer. %s\n", snd_strerror(rc));
		return -1;
	}

	rc = snd_mixer_attach(handle, card);
	if (rc < 0) {
		_log(LOG_ERR, "Could not attach to mixer to card. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_mixer_selem_register(handle, NULL, NULL);
	if (rc < 0) {
		_log(LOG_ERR, "Selem register failed. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_mixer_load(handle);
	if (rc < 0) {
		_log(LOG_ERR, "mixer load failded. %s\n", snd_strerror(rc));
		goto cleanup;
	};


	for (elem = snd_mixer_first_elem(handle); elem; elem = snd_mixer_elem_next(elem)) {
		const char* name;

		if (!snd_mixer_selem_is_active(elem))
			continue;
		
		if (snd_mixer_selem_has_playback_volume(elem)) {
			rc = snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
			if (rc == 0) {
				name = snd_mixer_selem_get_name(elem);
				vol = strcasestr(name, "master") ||
				      strcasestr(name, "out") ||
				      strcasestr(name, "speaker") ? max : min;

				rc = snd_mixer_selem_set_playback_volume_all(elem, vol);
				if (rc < 0)
					_log(LOG_ERR, "selem set playback volume failed. %s\n", snd_strerror(rc));
			} else
				_log(LOG_ERR, "selem get playback volume range failed. %s\n", snd_strerror(rc));
		}

		if (snd_mixer_selem_has_capture_volume(elem)) {
			rc = snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
			if (rc == 0) {
				rc = snd_mixer_selem_set_capture_volume_all(elem, max);
				if (rc < 0)
					_log(LOG_ERR, "selem set capture volume failed. %s\n", snd_strerror(rc));
			} else
				_log(LOG_ERR, "selem get capture volume range failed. %s\n", snd_strerror(rc));
		}
	}

cleanup:
	if (handle)
		snd_mixer_close(handle);

	return retval;
}

int alsa_init(const char* card, unsigned int rate, unsigned int channels, unsigned int bps, snd_pcm_t **handle) {
	int rc, retval = -2;
	snd_pcm_hw_params_t *params = 0;

	/* Open the PCM device in playback mode */
	rc = snd_pcm_open(handle, card, SND_PCM_STREAM_PLAYBACK, 0);
	if (rc < 0) {
		_log(LOG_ERR, "Can't open \"%s\" PCM device. %s\n", card, snd_strerror(rc));
		return -1;
	}

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(*handle, params);
	rc = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		_log(LOG_ERR, "Can't set interleaved mode. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params_set_rate_near(*handle, params, &rate, 0);
	if (rc < 0) {
		_log(LOG_ERR, "ERROR: Can't set rate. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params_set_channels(*handle, params, channels);
	if (rc < 0)  {
		_log(LOG_ERR, "Can't set channels number. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params_set_format(*handle, params,
			bps == 16 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8);
	if (rc < 0) {
		_log(LOG_ERR, "Can't set format. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params(*handle, params);
	if (rc < 0) {
		_log(LOG_ERR, "Can't set harware parameters. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_prepare (*handle);
	if (rc < 0) {
		fprintf (stderr, "Cannot prepare audio interface for use (%s)\n", snd_strerror (rc));
		goto cleanup;
	}

	return 0;

cleanup:
	snd_pcm_drain(*handle);
	snd_pcm_close(*handle);
	return retval;
}

__inline bool translate_frame(const CODE_FRAMEDEF *framedef, uint32_t *frame)
{
	const CODE_FRAME	*frame_table;

	for (frame_table = framedef->frames; frame_table < framedef->frames + framedef->frames_count; frame_table++)
		if (*frame == frame_table->code) {
			*frame = frame_table->frame;
			return true;
		}

	return false;
}

int play_rf_code(const char* card, const CODEDEF *code, uint64_t cmd, uint8_t repeat)
{
	uint8_t			*buffer = 0, *p;
	uint8_t			f, b, pl, ph, pulse_count;
	size_t			buf_size, l, r;
	unsigned int		rate = 48000, channels = 2, bps = 16;
	snd_pcm_uframes_t	pcm_frames;
	snd_pcm_t		*handle; 

	uint16_t		shead, ss, sl, strail;
	uint32_t		frame; /*unit, unit_mask; */
	int			rc, retval = -2;
	const CODE_FRAMEDEF	*framedef;

	sem_wait(&sem_sound);

	ss     = (uint16_t)round_div(code->short_len * rate, 1000000);
	sl     = (uint16_t)round_div(code->long_len  * rate, 1000000);
	strail = (uint16_t)round_div(code->trail_len * rate, 1000000);
	shead  = (uint16_t)round_div(code->head_len  * rate, 1000000);

	pulse_count = 0;
	framedef = code->framedef;
	for (f = code->cmd_len; f > 0; --f) {
		if (f < framedef->start)
			framedef++;
		pulse_count += framedef->frame_len;
	}

	buf_size = (shead + (sl * pulse_count) + strail) * channels * bps>>3;
	buffer =  (unsigned char*)malloc(buf_size);
	if (!buffer)
		return -1;

	p = buffer + buf_size;

	/* Generate code from the end */

	/* Trailer */
	for (l = strail<<1; l > 0; --l) {
		*(--p) = 0x7f;
		*(--p) = 0xff;
	}
	for (l = ss<<1; l > 0; --l) {
		*(--p) = 0x80;
		*(--p) = 0x00;
	}
	pcm_frames = strail + ss;

	ph = 0x7f;
	pl = 0xff;
	framedef = code->framedef;
	for (f = code->cmd_len; f > 0; --f) {
		if (f <= framedef->start)
			framedef++;

		frame = cmd & ((1 << framedef->code_len) - 1);
		cmd >>= framedef->code_len;

		if (!translate_frame(framedef, &frame)) {
			retval = -1;
			goto cleanup;
		}

		for (b = framedef->frame_len; b > 0; --b) {
			for (l = frame & 1 ? sl : ss; l > 0; --l) {
				*(--p) = ph;
				*(--p) = pl;
				*(--p) = ph;
				*(--p) = pl;
				pcm_frames++;
			}
			frame >>= 1;

			ph = ~ph;
			pl = ~pl;
		}
	}

	/* Header */
	if (shead) {
		for (l = shead<<1; l > 0; --l) {
			*(--p) = 0x7f;
			*(--p) = 0xff;
		}
		for (l = ss<<1; l > 0; --l) {
			*(--p) = 0x80;
			*(--p) = 0x00;
		}
		pcm_frames += shead + ss;
	}

	alsa_max_volume(pcm_device);

	if (alsa_init(card, rate, channels, bps, &handle))
		goto cleanup;

	for (r = 0; r < repeat; r++) {
		rc = snd_pcm_writei(handle, p, pcm_frames);
		if (rc < 0)
			_log(LOG_ERR, "Write %s\n", snd_strerror(rc));
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);

cleanup:
	if (buffer)
		free(buffer);

	sem_post(&sem_sound);
	return retval;
}

void rf_code_send(const CODEDEF *code, xPL_MessagePtr msg)
{
	char			buf[256];
	int			buf_pos;
	char			*repeat_str, *val_str;
	const CODE_FRAMEDEF	*framedef;
	uint64_t		val, cmd, mask;
	uint8_t			p, f, shift, len, repeat;

	buf_pos = snprintf(buf, sizeof(buf), "%s-%s.%s -> %s-%s.%s : %s.%s {",
		xPL_getSourceVendor(msg), xPL_getSourceDeviceID(msg), xPL_getSourceInstanceID(msg),
		xPL_getTargetVendor(msg), xPL_getTargetDeviceID(msg), xPL_getTargetInstanceID(msg),
		xPL_getSchemaClass(msg), xPL_getSchemaType(msg));

	cmd = code->init;

	for (p = 0; p < 4; p++) {
		const CODE_PARAM	*prm = code->param + p;
		const CODE_MAP		*map = prm->map;

		if (prm->len == 0)
			continue;

		if (0 == strcmp(prm->name, "id"))
			val_str = xPL_getTargetInstanceID(msg);
		else {
			val_str = xPL_getMessageNamedValue(msg, prm->name);
			buf_pos += snprintf(buf + buf_pos, sizeof(buf) - buf_pos, " %s=%s",
				prm->name, val_str);
		}

		if (!val_str)
			return;

		if (prm->map) {
			while (map->friendly && strcmp(val_str, map->friendly))
				map++;

			if (map->friendly == 0)
				map = 0;
		}

		if (map)
			val = map->value;
		else
			val = strtol(val_str, 0, 16);

		shift = 0;
		framedef = code->framedef;
		for (f = code->cmd_len; f > prm->start + prm->len; --f) {
			if (f <= framedef->start)
				framedef++;
			shift += framedef->code_len;
		}

		len = 0;
		for (; f > prm->start; --f) {
			if (f <= framedef->start)
				framedef++;
			len += framedef->code_len;
		}

		mask = prm->len == 64 ? -1 : (1 << len) - 1;
		cmd |= (val & mask)<<shift;
	}

	repeat_str = xPL_getMessageNamedValue(msg, "repeat");
	if (repeat_str)
		repeat = atoi(repeat_str);
	else
		repeat = 5;

	buf_pos += snprintf(buf + buf_pos, sizeof(buf) - buf_pos,
			" } => RF 433mhz code %s, %llX (repeat=%d)",
			code->name, cmd, repeat);
	_log(LOG_INFO, "%s", buf);
	play_rf_code(pcm_device, code, cmd, repeat);
}

void rf_code_msg_handler(xPL_MessagePtr msg, xPL_ObjectPtr data)
{
	char		*vendor, *device, *msg_class, *msg_type;
	const CODEDEF	*code;

	vendor = xPL_getTargetVendor(msg);
	if (vendor == 0 || strcmp(vendor, rf_code_vendor))
		return;

	device = xPL_getTargetDeviceID(msg);
	if (device == 0)
		return;

	msg_class = xPL_getSchemaClass(msg);
	if (msg_class == 0 || strcmp(msg_class, "control"))
		return;

	msg_type = xPL_getSchemaType(msg);
	if (msg_type == 0 || strcmp(msg_type, "basic"))
		return;


	for (code = rf433_codes; code < rf433_codes + rf433_codes_count; code++) {
		if (strcmp(device, code->name))
			continue;

		rf_code_send(code, msg);
	}
}

void xpl433mhz_shutdownHandler(int onSignal) {
	if (rf_code_service) {
		xPL_setServiceEnabled(rf_code_service, FALSE);
		xPL_releaseService(rf_code_service);
	}

	xPL_shutdown();
	_log(LOG_INFO, "Shutdown");
	exit(0);
}

int xpl433mhz_main(int argc, String argv[]) {
	if (!xPL_parseCommonArgs(&argc, argv, FALSE)) {
		_log(LOG_ERR, "Unable to start xPL");
		return -1;
	}

	if (!parse_args(argc, argv, 0, 0))
		return -2;

	/* Start xPL up */
	if (!xPL_initialize(xPL_getParsedConnectionType())) {
		_log(LOG_ERR, "Unable to start xPL");
		return -3;
	}

	sem_init(&sem_sound, 0, 1);

	rf_code_service =  xPL_createService((char*)rf_code_vendor, "sender", "default");
	xPL_setServiceVersion(rf_code_service, VERSION);
	xPL_setServiceEnabled(rf_code_service, TRUE);

	/* And a listener for all xPL messages */
	xPL_addMessageListener(rf_code_msg_handler, NULL);

	/* Install signal traps for proper shutdown */
	signal(SIGTERM, xpl433mhz_shutdownHandler);
	signal(SIGINT,  xpl433mhz_shutdownHandler);

	_log(LOG_INFO, "Startup");

	/** Main Loop  **/
	for (;;) {
	/* Let XPL run for a while, returning after it hasn't seen any */
	/* activity in 100ms or so                                     */
		xPL_processMessages(100);
	}
}
