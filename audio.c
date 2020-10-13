#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "mem.h"
#include "tinyalsa/asoundlib.h"
#include "audio.h"

struct audioS {
    struct pcm *pcm;
    size_t      buffer_size;
    audio_config_t cfg;
    audio_device_t dev;
    struct mixer     *mixer;
    struct mixer_ctl *out_volume, *in_volume;
    struct mixer_ctl *out_switch, *in_switch;
};

static void
find_mixer_controls(audio_t *audio)
{
    int n;

    audio->out_volume = audio->in_volume = NULL;
    audio->out_switch = audio->in_switch = NULL;

    n = mixer_get_num_ctls(audio->mixer);
    for (int i = 0; i < n; i++) {
        struct mixer_ctl *ctl = mixer_get_ctl(audio->mixer, i);
	if (ctl) {
	    const char *name = mixer_ctl_get_name(ctl);
	    if (! audio->out_volume && strstr(name, "Playback Volume") != NULL) {
		audio->out_volume = ctl;
	    }
	    if (! audio->in_volume && strstr(name, "Capture Volume") != NULL) {
		audio->out_volume = ctl;
	    }
	    if (! audio->out_switch && strstr(name, "Playback Switch") != NULL) {
		audio->out_volume = ctl;
	    }
	    if (! audio->out_switch && strstr(name, "PCM Playback Route") != NULL) {
		audio->out_volume = ctl;
	    }
	    if (! audio->in_switch && strstr(name, "Capture Switch") != NULL) {
		audio->out_volume = ctl;
	    }
	}
    }
}

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
    c-> mixer = mixer_open(c->dev.card);

    find_mixer_controls(c);

    return c;
}

size_t
audio_get_buffer_size(audio_t *c)
{
    return c->buffer_size;
}

static bool
set_switch(struct mixer_ctl *sw, int value)
{
    if (sw == NULL) return true;

    for (int i = 0; i < mixer_ctl_get_num_values(sw); i++) {
        mixer_ctl_set_value(sw, i, value);
    }

    return true;
}

static bool
set_volume(struct mixer_ctl *ctl, unsigned volume)
{
    for (int i = 0; i < mixer_ctl_get_num_values(ctl); i++) {
        mixer_ctl_set_percent(ctl, i, volume);
    }

    return true;
}

bool
audio_set_volume(audio_t *audio, unsigned volume)
{
    bool res = true;
    bool playback = audio->dev.playback;

    res &= set_volume(audio->out_volume, playback ? volume : 0);
    res &= set_volume(audio->in_volume,  playback ? 0 : volume);
    res &= set_switch(audio->out_switch, playback ? 1 : 0);
    res &= set_switch(audio->in_switch,  playback ? 0 : 1);

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

bool
audio_print_controls(audio_t *audio, FILE *f)
{
    int n;

    n = mixer_get_num_ctls(audio->mixer);
    for (int i = 0; i < n; i++) {
        struct mixer_ctl *ctl = mixer_get_ctl(audio->mixer, i);
	if (ctl) {
	    printf("%d. %s\n", i, mixer_ctl_get_name(ctl));
	}
    }

    return true;
}

void
audio_destroy(audio_t *c)
{
    mixer_close(c->mixer);
    pcm_close(c->pcm);
    free(c);
}
