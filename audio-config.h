#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

#include "util.h"

typedef struct {
    unsigned channels;
    unsigned rate;
    unsigned bits;
} audio_config_t;

typedef struct {
    unsigned card;
    unsigned device;
    bool     playback;
} audio_device_t;

typedef struct {
     unsigned sample_rate;
     unsigned num_channels;
     unsigned bytes_per_sample;
} audio_meta_t;

static inline void
audio_config_init_default(audio_config_t *c)
{
    c->channels = 2;
    c->rate = 44100;
    c->bits = 16;
}

static inline void
audio_config_print(audio_config_t *cfg, FILE *f)
{
    fprintf(f, "cfg[bits=%d, channels=%d, rate=%d]", cfg->bits, cfg->channels, cfg->rate);
}

static inline void
audio_device_init(audio_device_t *d, unsigned card, unsigned device, bool playback)
{
    d->card = card;
    d->device = device;
    d->playback = playback;
}

static inline void
audio_device_init_playback(audio_device_t *d)
{
    audio_device_init(d, 0, 0, true);
}

static inline void
audio_device_init_capture(audio_device_t *d)
{
    audio_device_init(d, 1, 0, false);
}

static inline void
audio_meta_init(audio_meta_t *m, unsigned sample_rate, unsigned num_channels, unsigned bytes_per_sample)
{
     m->sample_rate = sample_rate;
     m->num_channels = num_channels;
     m->bytes_per_sample = bytes_per_sample;
}

static inline void
audio_meta_init_from_config(audio_meta_t *m, audio_config_t *c)
{
    m->sample_rate = c->rate;
    m->num_channels = c->channels;
    m->bytes_per_sample = (c->bits+7)/8;
}

#endif

