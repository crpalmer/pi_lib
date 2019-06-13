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
    audio_device_t dev;
};

audio_t *
audio_new(audio_config_t *cfg, audio_device_t *device)
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

    if ((pcm = pcm_open(device->card, device->device, device->playback ? PCM_OUT : PCM_IN, &config)) == NULL ||
	! pcm_is_ready(pcm))
    {
	fprintf(stderr, "Failed to open: %s\n", pcm_get_error(pcm));
	return NULL;
    }

    c = (audio_t *) fatal_malloc(sizeof(*c));
    c->pcm = pcm;
    c->cfg = *cfg;
    c->dev = *device;
    c->buffer_size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));

    return c;
}

size_t
audio_get_buffer_size(audio_t *c)
{
    return c->buffer_size;
}

static bool
mixer_set(struct mixer *mixer, const char *name, int value)
{
    struct mixer_ctl *ctl;
    unsigned i;

    ctl = mixer_get_ctl_by_name(mixer, name);
    if (! ctl) {
	fprintf(stderr, "Failed to find control: %s\n", name);
	mixer_close(mixer);
	return false;
    }

    for (i = 0; i < mixer_ctl_get_num_values(ctl); i++) {
        mixer_ctl_set_value(ctl, i, value);
    }

    return true;
}

static bool
do_set_volume(struct mixer *mixer, const char *name, unsigned volume)
{
    struct mixer_ctl *ctl;
    unsigned i;

    ctl = mixer_get_ctl_by_name(mixer, name);
    if (! ctl) {
	fprintf(stderr, "Failed to find control: %s\n", name);
	mixer_close(mixer);
	return false;
    }

    for (i = 0; i < mixer_ctl_get_num_values(ctl); i++) {
        mixer_ctl_set_percent(ctl, i, volume);
    }

    return true;
}

bool
audio_set_volume(audio_t *audio, unsigned volume)
{
    struct mixer *mixer;
    bool res;

    mixer = mixer_open(audio->dev.card);
    if (! mixer) {
	fprintf(stderr, "Failed to open mixer\n");
	return false;
    }

    if (audio->dev.playback) {
        res = do_set_volume(mixer, "PCM Playback Volume", volume) ||
	      mixer_set(mixer, "PCM Playback Route", 1);
    } else {
        res = do_set_volume(mixer, "Mic Capture Volume", volume) ||
	      mixer_set(mixer, "Mic Capture Switch", 1) ||
	      mixer_set(mixer, "Speaker Playback Switch", 0) ||
	      mixer_set(mixer, "Mic Playback Switch", 0);
    }

    mixer_close(mixer);

    return res;
}

bool
audio_capture_buffer(audio_t *c, unsigned char *buffer)
{
    return pcm_read(c->pcm, buffer, c->buffer_size) == 0;
}

bool
audio_play_buffer(audio_t *c, const unsigned char *buffer, size_t size)
{
    return pcm_write(c->pcm, buffer, size) == 0;
}

void
audio_destroy(audio_t *c)
{
    pcm_close(c->pcm);
    free(c);
}
