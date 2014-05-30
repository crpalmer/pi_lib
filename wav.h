#ifndef __WAV_H__
#define __WAV_H__

#include "util.h"
#include "audio.h"

typedef struct wavS wav_t;

typedef void (*wav_servo_update_t)(void *data, double pos);

wav_t *
wav_new(const char *fname);

wav_t *
wav_new_with_servo_track(const char *fname, wav_servo_update_t fn, void *fn_data);

void
wav_generate_servo_data(wav_t *w, wav_servo_update_t fn, void *fn_data);

void
wav_configure_audio(wav_t *w, audio_config_t *m);

bool
wav_play(wav_t *w, audio_t *audio);

void
wav_destroy(wav_t *w);

#endif
