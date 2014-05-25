#ifndef __WAV_H__
#define __WAV_H__

#include "util.h"

typedef struct wavS wav_t;

typedef void (*wav_servo_update_t)(void *data, double pos);

wav_t *
wav_new(const char *fname);

wav_t *
wav_new_with_servo_track(const char *fname, wav_servo_update_t fn, void *fn_data);

bool
wav_set_volume(unsigned volume);

bool
wav_play(wav_t *w);

void
wav_destroy(wav_t *w);

#endif
