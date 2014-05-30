#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "mem.h"
#include "tinyalsa/asoundlib.h"
#include "audio.h"

struct audioS {
    struct pcm *pcm;
    size_t      buffer_size;
    audio_config_t cfg;
};

static audio_t *
do_audio_new(audio_config_t *cfg, unsigned which, audio_device_t *device)
{
    struct pcm_config config;
    struct pcm *pcm;
    audio_t *c;

    if (cfg->bits != 16 && cfg->bits != 24 && cfg->bits != 32) {
	errno = EINVAL;
	return NULL;
    }

    config.channels = cfg->channels;
    config.rate = cfg->rate;
    config.period_size = 1024;
    config.period_count = 4;
    config.format = cfg->bits == 16 ? PCM_FORMAT_S16_LE : cfg->bits == 32 ? PCM_FORMAT_S32_LE : PCM_FORMAT_S24_LE;
    config.start_threshold = config.stop_threshold = config.silence_threshold = 0;

    if ((pcm = pcm_open(device->card, device->device, which, &config)) == NULL ||
	! pcm_is_ready(pcm))
    {
	return NULL;
    }

    c = fatal_malloc(sizeof(*c));
    c->pcm = pcm;
    c->cfg = *cfg;
    c->buffer_size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));

    return c;
}

audio_t *
audio_new_capture(audio_config_t *cfg, audio_device_t *device)
{
    return do_audio_new(cfg, PCM_IN, device);
}

audio_t *
audio_new_playback(audio_config_t *cfg, audio_device_t *device)
{
    return do_audio_new(cfg, PCM_OUT, device);
}

size_t
audio_get_buffer_size(audio_t *c)
{
    return c->buffer_size;
}

bool
audio_capture_buffer(audio_t *c, unsigned char *buffer)
{
    return pcm_read(c->pcm, buffer, c->buffer_size) == 0;
}

bool
audio_play_buffer(audio_t *c, const unsigned char *buffer)
{
    return pcm_write(c->pcm, buffer, c->buffer_size) == 0;
}

void
audio_destroy(audio_t *c)
{
    pcm_close(c->pcm);
    free(c);
}
