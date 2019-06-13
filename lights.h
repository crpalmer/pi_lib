#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "piface.h"

typedef struct lightsS lights_t;

lights_t *
lights_new(piface_t *piface);

void
lights_chase(lights_t *lights);

void
lights_on(lights_t *lights);

void
lights_off(lights_t *lights);

void
lights_select(lights_t *lights, unsigned selected);

void
lights_blink(lights_t *lights);

void
lights_destroy(lights_t *lights);

#ifdef __cplusplus
};
#endif

#endif
