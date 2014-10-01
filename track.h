#ifndef __TRACK_H__
#define __TRACK_H__

typedef struct trackS track_t;

track_t *
track_new(const char *fname);

void
track_play(track_t *t);

void
track_destroy(track_t *);

#endif
