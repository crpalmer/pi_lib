#ifndef __TRACK_H__
#define __TRACK_H__

typedef struct trackS track_t;

track_t *
track_new(const char *fname);

void
track_set_volume(track_t *t, unsigned volume);

void
track_play(track_t *t);

void
track_play_with_stop_needed(track_t *t, volatile bool *stop_needed);

void
track_play_asynchronously(track_t *t, volatile bool *stop_needed);

void
track_destroy(track_t *);

#endif
