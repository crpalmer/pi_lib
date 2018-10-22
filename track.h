#ifndef __TRACK_H__
#define __TRACK_H__

typedef struct trackS track_t;

#include <stdbool.h>
#include "audio-config.h"
#include "stop.h"

track_t *
track_new(const char *fname);

track_t *
track_new_audio_dev(const char *fname, audio_device_t *dev);

track_t *
track_new_fatal(const char *fname);

track_t *
track_new_audio_dev_fatal(const char *fname, audio_device_t *dev);

track_t *
track_new(const char *fname);

void
track_set_volume(track_t *t, unsigned volume);

void
track_play(track_t *t);

void
track_play_with_stop(track_t *t, stop_t *stop);

void
track_play_asynchronously(track_t *t, stop_t *stop);

void
track_play_loop(track_t *t, stop_t *stop);

void
track_destroy(track_t *);

#endif
