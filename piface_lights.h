#ifndef __PIFACE_LIGHTS_H__
#define __PIFACE_LIGHTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "piface.h"

typedef struct piface_lightsS piface_lights_t;

piface_lights_t *
piface_lights_new(piface_t *piface);

void
piface_lights_chase(piface_lights_t *lights);

void
piface_lights_on(piface_lights_t *lights);

void
piface_lights_off(piface_lights_t *lights);

void
piface_lights_select(piface_lights_t *lights, unsigned selected);

void
piface_lights_blink(piface_lights_t *lights);

void
piface_lights_destroy(piface_lights_t *lights);

#ifdef __cplusplus
};
#endif

#endif
