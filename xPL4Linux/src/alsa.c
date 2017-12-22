#define _GNU_SOURCE 
#include <string.h>
#include "alsa.h"
#include "log.h"

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

int alsa_init_playback(const char* card, unsigned int *rate, unsigned int channels, unsigned int bps, snd_pcm_t **handle)
{
	int			rc, retval = -2;
	snd_pcm_hw_params_t	*params = 0;

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

	rc = snd_pcm_hw_params_set_rate_near(*handle, params, rate, 0);
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
		_log(LOG_ERR, "Can't set hardware parameters. %s\n", snd_strerror(rc));
		goto cleanup;
	}

	rc = snd_pcm_prepare (*handle);
	if (rc < 0) {
		_log(LOG_ERR, "Cannot prepare audio interface for use (%s)\n", snd_strerror (rc));
		goto cleanup;
	}

	return 0;

cleanup:
	snd_pcm_drain(*handle);
	snd_pcm_close(*handle);
	return retval;
}

int alsa_init_capture(const char* card, unsigned int *rate, unsigned int channels, unsigned int bps, snd_pcm_t **handle)
{
	int rc, retval = -2;
	snd_pcm_hw_params_t *params = 0;

	/* Open the PCM device in capture mode */
	rc = snd_pcm_open(handle, card, SND_PCM_STREAM_CAPTURE, 0);
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

	rc = snd_pcm_hw_params_set_rate_near(*handle, params, rate, 0);
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
		_log(LOG_ERR, "Cannot prepare audio interface for use (%s)\n", snd_strerror (rc));
		goto cleanup;
	}

	return 0;

cleanup:
	snd_pcm_drain(*handle);
	snd_pcm_close(*handle);
	return retval;
}
