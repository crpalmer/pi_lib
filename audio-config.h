#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

typedef struct {
    unsigned channels;
    unsigned rate;
    unsigned bits;
} audio_config_t;

typedef struct {
    unsigned card;
    unsigned device;
} audio_device_t;

static inline void
audio_config_init_default(audio_config_t *c)
{
    c->channels = 2;
    c->rate = 44100;
    c->bits = 16;
}

static inline void
audio_device_init(audio_device_t *d, unsigned card, unsigned device)
{
    d->card = card;
    d->device = device;
}

static inline void
audio_device_init_playback(audio_device_t *d)
{
    audio_device_init(d, 0, 0);
}

static inline void
audio_device_init_capture(audio_device_t *d)
{
    audio_device_init(d, 1, 0);
}

#endif

