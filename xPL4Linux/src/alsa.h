#include <alsa/asoundlib.h>

int alsa_max_volume(const char* card);
int alsa_init_playback(const char* card, unsigned int *rate, unsigned int channels, unsigned int bps, snd_pcm_t **handle);
int alsa_init_capture(const char* card, unsigned int *rate, unsigned int channels, unsigned int bps, snd_pcm_t **handle);
