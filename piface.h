#ifndef __PIFACE_H__
#define __PIFACE_H__

#include "util.h"

typedef struct pifaceS piface_t;

piface_t *
piface_new(void);

piface_t *
piface_new_id(unsigned id);

void
piface_set(piface_t *p, unsigned pin, bool value);

bool
piface_get(piface_t *p, unsigned pin);

unsigned
piface_get_all(piface_t *p);

void
piface_wait_for_input(piface_t *p);

void
piface_destroy(piface_t *p);

#endif