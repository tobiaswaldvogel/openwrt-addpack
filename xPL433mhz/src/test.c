#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <alsa/asoundlib.h>

#define VERSION "1.0"

static const char pcm_device[] = "default";


int alsa_max_volume(const char* card)
{
	int rc, retval = 2;
	long min, max, vol;
	snd_mixer_t *handle = 0;
	snd_mixer_elem_t *elem;

	rc = snd_mixer_open(&handle, 0);
	if (rc < 0) {
		fprintf(stderr, "Can't open mixer. %s\n", snd_strerror(rc));
		return -1;
	}

	rc = snd_mixer_attach(handle, card);
	if (rc < 0) {
		fprintf(stderr, "Could not attach to mixer to card. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_mixer_selem_register(handle, NULL, NULL);
	if (rc < 0) {
		fprintf(stderr, "Selem register failed. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_mixer_load(handle);
	if (rc < 0) {
		fprintf(stderr, "mixer load failded. %s\n", snd_strerror(rc));
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
					fprintf(stderr, "selem set playback volume failed. %s\n", snd_strerror(rc));
			} else
				fprintf(stderr, "selem get playback volume range failed. %s\n", snd_strerror(rc));
		}

		if (snd_mixer_selem_has_capture_volume(elem)) {
			rc = snd_mixer_selem_get_capture_volume_range(elem, &min, &max);
			if (rc == 0) {
				rc = snd_mixer_selem_set_capture_volume_all(elem, max);
				if (rc < 0)
					fprintf(stderr, "selem set capture volume failed. %s\n", snd_strerror(rc));
			} else
				fprintf(stderr, "selem get capture volume range failed. %s\n", snd_strerror(rc));
		}
	}

cleanup:

	if (handle)
		snd_mixer_close(handle);

	return retval;
}

int alsa_init_capture(const char* card, unsigned int rate, unsigned int channels, unsigned int bps, snd_pcm_t **handle) {
	int rc, retval = -2;
	snd_pcm_hw_params_t *params = 0;

	/* Open the PCM device in capture mode */
	rc = snd_pcm_open(handle, card, SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0) {
		fprintf(stderr, "Can't open \"%s\" PCM device. %s\n", card, snd_strerror(rc));
		return -1;
	}

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(*handle, params);
	rc = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (rc < 0) {
		fprintf(stderr, "Can't set interleaved mode. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params_set_rate_near(*handle, params, &rate, 0);
	if (rc < 0) {
		fprintf(stderr, "ERROR: Can't set rate. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params_set_channels(*handle, params, channels);
	if (rc < 0)  {
		fprintf(stderr, "Can't set channels number. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params_set_format(*handle, params,
			bps == 16 ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8);
	if (rc < 0) {
		fprintf(stderr, "Can't set format. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_hw_params(*handle, params);
	if (rc < 0) {
		fprintf(stderr, "Can't set harware parameters. %s\n", snd_strerror(rc));
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
/*
int play_cmd(const char* card, const CMD *cmd)
{
	unsigned char		*buffer = 0, *p, *code, bit, val, pl, ph;
	size_t			buf_size, s, c, l, r;
	unsigned int		rate = 48000, channels = 2, bps = 16;
	unsigned int		pulse = 0x7fff;
	snd_pcm_uframes_t	frames = 0;
	snd_pcm_uframes_t	frames_sync = 0;
	snd_pcm_t		*handle; 

	unsigned int		ssync = cmd->sync * rate / 1000000;
	unsigned int		ss = cmd->s * rate / 1000000;
	unsigned int		sl = cmd->l * rate / 1000000;
	int			rc, retval = -2;

	buf_size = (cmd->sync + (cmd->l * cmd->len)) * channels * bps>>3;
	buffer =  (unsigned char*)malloc(buf_size);
	if (!buffer)
		return -1;

	p = buffer;
	pl = pulse & 0xff;
	ph = pulse >> 8;;

	for (s = 0; s < ssync; s++) {
		*(p++) = pl;
		*(p++) = ph;
		*(p++) = pl;
		*(p++) = ph;
		frames++;
	}
	frames_sync = frames;

	code = cmd->code;

	switch (cmd->ctype) {
	case 0:
		bit = 0;
		for (c = 0; c < cmd->len; c++) {
			pulse = ~pulse;
			pl = pulse & 0xff;
			ph = pulse >> 8;;

			if (bit == 0) {
				bit = 0x80;
				val = *(code++);
			}

			for (l = val & bit ? sl : ss; l > 0; --l) {
				*(p++) = pl;
				*(p++) = ph;
				*(p++) = pl;
				*(p++) = ph;
				frames++;
			}

			bit >>= 1;
		}
		break;

	case 1:
		bit = 0;
		for (c = 0; c < cmd->len; c++) {
			if (bit == 0) {
				bit = 0x80;
				val = *(code++);
			}

			pulse = ~pulse;
			pl = pulse & 0xff;
			ph = pulse >> 8;;

			for (l = val & bit ? sl : ss; l > 0; --l) {
				*(p++) = pl;
				*(p++) = ph;
				*(p++) = pl;
				*(p++) = ph;
				frames++;
			}

			pulse = ~pulse;
			pl = pulse & 0xff;
			ph = pulse >> 8;;

			for (l = val & bit ? ss : sl; l > 0; --l) {
				*(p++) = pl;
				*(p++) = ph;
				*(p++) = pl;
				*(p++) = ph;
				frames++;
			}
			bit >>= 1;
		}
		break;
	}

	alsa_max_volume(pcm_device);

	if (alsa_init(card, rate, channels, bps, &handle))
		goto cleanup;

	for (r = 0; r < cmd->repeat; r++) {
		rc = snd_pcm_writei(handle, buffer, frames);
		if (rc < 0)
			fprintf(stderr, "Write %s\n", snd_strerror(rc));
	}

	rc = snd_pcm_writei(handle, buffer, frames_sync);
	if (rc < 0)
		fprintf(stderr, "Write %s\n", snd_strerror(rc));

	snd_pcm_drain(handle);
	snd_pcm_close(handle);

cleanup:
	if (buffer)
		free(buffer);
	return retval;
}


int usage() {
	return -1;
	fprintf(stderr, "%s", "Usage: \n\n");
}

int add_code(char* cmd)
{
	char	hex[3], *p;
	uint8_t	*code;
	size_t	pos;
	int	i, len;

	for (pos=0; cmd[pos] && cmd[pos] != '-'; pos++);
	if (!cmd[pos])
		return -1;
	cmds[cmd_count].vendor = strndup(cmd,pos);
	cmd += pos + 1;

	for (pos=0; cmd[pos] && cmd[pos] != '.'; pos++);
	if (!cmd[pos])
		return -2;
	cmds[cmd_count].device = strndup(cmd,pos);
	cmd += pos + 1;

	for (pos=0; cmd[pos] && cmd[pos] != ':'; pos++);
	if (!cmd[pos])
		return -3;
	cmds[cmd_count].instance = strndup(cmd,pos);
	cmd += pos + 1;

	for (pos=0; cmd[pos] && cmd[pos] != '='; pos++);
	if (!cmd[pos])
		return -4;
	cmds[cmd_count].cmd = strndup(cmd,pos);
	cmd += pos + 1;

	cmds[cmd_count].repeat = strtol(cmd, &p, 0);
	if (cmd == p || *p != ',')
		return -5;
	cmd = p + 1;

	cmds[cmd_count].sync = strtol(cmd, &p, 0);
	if (cmd == p || *p != ',')
		return -6;
	cmd = p + 1;

	cmds[cmd_count].s = strtol(cmd, &p, 0);
	if (cmd == p || *p != ',')
		return -7;
	cmd = p + 1;

	cmds[cmd_count].l = strtol(cmd, &p, 0);
	if (cmd == p || *p != ',')
		return -8;
	cmd = p + 1;

	hex[0] = *(cmd++);
	hex[1] = 0;
	cmds[cmd_count].ctype = strtol(hex, &p, 16);
	if (p != hex + 1)
		return -9;

	hex[0] = *(cmd++);
	if (!*cmd)
		return -10;
	hex[1] = *(cmd++);
	hex[2] = 0;
	len = strtol(hex, &p, 16);
	if (p != hex + 2)
		return -10;

	if (len >  256)
		return -10;

	code = malloc(1 + ((len+7)/8));

	cmds[cmd_count].len = len;
	cmds[cmd_count].code = code;

	code = cmds[cmd_count].code;
	for (i=0; i<len; i += 8) {
		if (!*cmd) {
			if (i + 8 < len)
				return -11;
			else {
				*code = 0;
				continue;
			}
		}

		hex[0] = *(cmd++);
		hex[1] = *(cmd++);
		if (!hex[1])
			return -11;
		
		hex[2] = 0;
		*(code++) = strtol(hex, &p, 16);
		if (p != hex+2)
			return -11;
	}

	if (*cmd)
		return -12;

	cmd_count++;
	return 0;
}
*/

snd_pcm_t		*capture_dev = 0; 


void shutdownHandler(int onSignal) {
	if (capture_dev) {
		snd_pcm_drain(capture_dev);
		snd_pcm_close(capture_dev);
	}

	exit(0);
}

#define NOISE_LIMIT 100
#define MIN_SYNC_LEN 3000

int capture(const char* card)
{
	unsigned int		rate = 48000, channels = 1, bps = 16;
	unsigned int		frame_size = channels * (bps == 16 ? 2 : 1);
	snd_pcm_uframes_t	frames, buf_size = rate/ 10;
	unsigned char		buffer[buf_size * frame_size];

	uint32_t	min_sync_len = MIN_SYNC_LEN * rate / 1000000;
	uint32_t	noise_limit = NOISE_LIMIT * rate / 1000000;

	fprintf(stderr, "min_sync_len %u, noise_limit %u\n", min_sync_len, noise_limit);


	if (alsa_init_capture(card, rate, channels, bps, &capture_dev))
		return -1;

	for (;;) {
		frames = snd_pcm_readi(capture_dev, buffer, buf_size);
		if ((long)frames < 0) {
			snd_pcm_state_t status;
			status = snd_pcm_state(capture_dev);
			fprintf(stderr, "pcm read error %s  %d\n", snd_strerror(frames), status);
			break;
		}
//		fprintf(stderr, "frames %lu %lu\n", frames, buf_size);
	}
	return 0;
}

int main(int argc, char* argv[]) {
	signal(SIGTERM, shutdownHandler);
	signal(SIGINT, shutdownHandler);


	alsa_max_volume(pcm_device);
	capture(pcm_device);
	shutdownHandler(9);
}

