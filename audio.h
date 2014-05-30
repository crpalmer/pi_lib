#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "audio-config.h"
#include "util.h"

typedef struct audioS audio_t;

audio_t *
audio_new(audio_config_t *cfg, audio_device_t *device);

size_t
audio_get_buffer_size(audio_t *);

bool
audio_set_volume(audio_t *, unsigned vol);

bool
audio_capture_buffer(audio_t *, unsigned char *buffer);

bool
audio_play_buffer(audio_t *, const unsigned char *buffer, size_t size);

void
audio_destroy(audio_t *);

#endif
