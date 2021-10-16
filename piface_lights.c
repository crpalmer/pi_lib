#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include "mem.h"
#include "util.h"

#include "piface_lights.h"

typedef enum { NONE, CHASE, ON, OFF, SELECTED, BLINK, EXIT } action_t;

struct piface_lightsS {
    piface_t    *piface;

    pthread_t    thread;
    pthread_mutex_t lock;
    pthread_cond_t  cond;

    action_t     action;
    unsigned	 selected;
    size_t	 next_chase;
    bool         blink_on;
};

#define ANIMATION_SLEEP_MS	200
#define LIGHT_0	 2
#define N_LIGHTS 5

unsigned chase_seq[] = { 0, 1, 4, 2, 3, 4 };
#define N_CHASE_SEQ (sizeof(chase_seq) / sizeof(chase_seq[0]))

static void
set(piface_lights_t *lights, unsigned id, bool value)
{
    piface_set(lights->piface, id + LIGHT_0, value);
}

static void
set_all(piface_lights_t *lights, bool value)
{
    size_t i;

    for (i = 0; i < N_LIGHTS; i++) set(lights, i, value);
}

static void
do_chase_step(piface_lights_t *lights)
{
    set_all(lights, false);
    set(lights, chase_seq[lights->next_chase], true);
    lights->next_chase = (lights->next_chase + 1) % N_CHASE_SEQ;
    set(lights, chase_seq[lights->next_chase], true);
}

static void
do_selected(piface_lights_t *lights)
{
    set_all(lights, false);
    set(lights, lights->selected, true);
}

static void *
piface_lights_work(void *lights_as_vp)
{
    piface_lights_t *lights = (piface_lights_t *) lights_as_vp;

    for (;;) {
	pthread_mutex_lock(&lights->lock);
	while (lights->action == NONE) {
	    pthread_cond_wait(&lights->cond, &lights->lock);
	}
	switch (lights->action) {
	case NONE:
	    break;
	case CHASE:
	    do_chase_step(lights);
	    break;
	case ON:
	    set_all(lights, true);
	    lights->action = NONE;
	    break;
	case OFF:
	    set_all(lights, false);
	    lights->action = NONE;
	    break;
	case SELECTED:
	    do_selected(lights);
	    lights->action = NONE;
	    break;
	case BLINK:
	    lights->blink_on = !lights->blink_on;
	    set_all(lights, lights->blink_on);
	    break;
	case EXIT:
	    return NULL;
	}
	pthread_mutex_unlock(&lights->lock);
	if (lights->action == CHASE || lights->action == BLINK) {
	    ms_sleep(ANIMATION_SLEEP_MS);
	}
    }
}

piface_lights_t *
piface_lights_new(piface_t *piface)
{
    piface_lights_t *lights = fatal_malloc(sizeof(*lights));

    lights->piface = piface;

    lights->action = NONE;
    lights->selected = 0;
    lights->next_chase = 0;
    lights->blink_on = false;

    pthread_mutex_init(&lights->lock, NULL);
    pthread_cond_init(&lights->cond, NULL);

    pthread_create(&lights->thread, NULL, piface_lights_work, lights);

    return lights;
}

#define SELECTED_NONE ((unsigned) -1)

static void
send_work_selected(piface_lights_t *lights, action_t action, unsigned selected)
{
    pthread_mutex_lock(&lights->lock);
    /* Once exit is picked, never override it! */
    if (lights->action != EXIT) {
        lights->action = action;
        if (selected != SELECTED_NONE) lights->selected = selected;
    }
    pthread_cond_signal(&lights->cond);
    pthread_mutex_unlock(&lights->lock);
}

static void
send_work(piface_lights_t *lights, action_t action)
{
    send_work_selected(lights, action, SELECTED_NONE);
}

void
piface_lights_chase(piface_lights_t *lights)
{
    send_work(lights, CHASE);
}

void
piface_lights_on(piface_lights_t *lights)
{
    send_work(lights, ON);
}

void
piface_lights_off(piface_lights_t *lights)
{
    send_work(lights, OFF);
}

void
piface_lights_select(piface_lights_t *lights, unsigned selected)
{
    send_work_selected(lights, SELECTED, selected);
}

void
piface_lights_blink(piface_lights_t *lights)
{
    send_work(lights, BLINK);
}

void
piface_lights_destroy(piface_lights_t *lights)
{
    send_work(lights, EXIT);
    pthread_join(lights->thread, NULL);

    pthread_mutex_destroy(&lights->lock);
    pthread_cond_destroy(&lights->cond);
    free(lights);
}
