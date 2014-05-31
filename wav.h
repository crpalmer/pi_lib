#ifndef __WAV_H__
#define __WAV_H__

#include "talking-skull.h"
#include "util.h"

#include "audio.h"

typedef struct wavS wav_t;

wav_t *
wav_new(const char *fname);

talking_skull_t *
wav_extract_servo_track(wav_t *w);

talking_skull_t *
wav_generate_servo_data(wav_t *w);

void
wav_configure_audio(wav_t *w, audio_config_t *m);

bool
wav_play(wav_t *w, audio_t *audio);

void
wav_destroy(wav_t *w);

#endif
