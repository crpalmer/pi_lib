#ifndef __TALKING_SKULL__
#define __TALKING_SKULL__

#include "audio-config.h"

typedef struct talking_skullS talking_skull_t;

typedef void (*talking_skull_servo_update_t)(void *data, double pos);

talking_skull_t *
talking_skull_new(audio_meta_t *m, bool is_track, talking_skull_servo_update_t fn, void *fn_data);

talking_skull_t *
talking_skull_new_with_n_to_avg(audio_meta_t *m, size_t n_to_avg, talking_skull_servo_update_t fn, void *fn_data);

void
talking_skull_destroy(talking_skull_t *t);

unsigned
talking_skull_play(talking_skull_t *t, unsigned char *data, unsigned n_bytes);

void
talking_skull_wait_completion(talking_skull_t *t, unsigned handle);

#endif
