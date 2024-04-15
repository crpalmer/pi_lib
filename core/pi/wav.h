#ifndef __WAV_H__
#define __WAV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stop.h"
#include "talking-skull.h"
#include "util.h"

#include "audio.h"

typedef struct wavS wav_t;

wav_t *
wav_new(const char *fname);

void
wav_extract_servo_track(wav_t *w);

void
wav_configure_audio(wav_t *w, audio_config_t *m);

audio_meta_t
wav_get_meta(wav_t *w);

unsigned char *
wav_get_raw_data(wav_t *w, size_t *n_ret);

bool
wav_play(wav_t *w, audio_t *audio);

bool
wav_play_with_stop(wav_t *w, audio_t *audio, stop_t *stop);

bool
wav_play_with_talking_skull(wav_t *w, audio_t *audio, talking_skull_t *);

void
wav_destroy(wav_t *w);

#ifdef __cplusplus
};
#endif

#endif
