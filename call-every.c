#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "time-utils.h"
#include "mem.h"
#include "call-every.h"

#define TRACE 0

typedef enum {
    RUN, PAUSE, STOP
} call_every_state_t;

struct call_everyS {
    unsigned           ms;
    call_every_func_t  func;
    void              *data;
    call_every_state_t state;
#ifdef PI_PICO
    repeating_timer_t  timer;
#else
    pthread_t          thread;
    pthread_mutex_t    lock;
    pthread_cond_t     cond;
#endif
};

#ifdef PI_PICO

static bool
callback(repeating_timer_t *t)
{
    call_every_t *e = (call_every_t *) t->user_data;
    e->func(e->data);
    return true;
}

#else

static void *
thread_main(void *e_as_vp)
{
    call_every_t *e = (call_every_t *) e_as_vp;
    struct timespec last;

    nano_gettime(&last);

    pthread_mutex_lock(&e->lock);

    while (e->state != STOP) {
	if (e->state == PAUSE) {
	    pthread_cond_wait(&e->cond, &e->lock);
	    nano_gettime(&last);
	} else {
	    pthread_mutex_unlock(&e->lock);
	    nano_add_ms(&last, e->ms);
	    nano_sleep_until(&last);
	    if (TRACE) {
		struct timespec now;
		nano_gettime(&now);
		fprintf(stderr, "ce at: " TIMESPEC_FMT " want " TIMESPEC_FMT "\n", (int) now.tv_sec, (int) now.tv_nsec, (int) last.tv_sec, (int) last.tv_nsec);
	    }
	    e->func(e->data);
	    pthread_mutex_lock(&e->lock);
	}
    }

    return NULL;
}

#endif

call_every_t *
call_every_new(unsigned ms, call_every_func_t func, void *data)
{
    call_every_t *e;

    assert(func != NULL);

    e = fatal_malloc(sizeof(*e));
    e->ms   = ms;
    e->func = func;
    e->data = data;

#ifndef PI_PICO
    e->state = PAUSE;

    pthread_mutex_init(&e->lock, NULL);
    pthread_cond_init(&e->cond, NULL);

    pthread_create(&e->thread, NULL, thread_main, e);
#endif

    return e;
}

static void
set_state(call_every_t *e, call_every_state_t state)
{
#ifdef PI_PICO
    if (state == RUN) {
	alarm_pool_add_repeating_timer_ms(alarm_pool_get_default(), e->ms, callback, e, &e->timer);
    } else {
	cancel_repeating_timer(&e->timer);
    }
#else
    assert(e);

    pthread_mutex_lock(&e->lock);
    e->state = state;
    pthread_cond_signal(&e->cond);
    pthread_mutex_unlock(&e->lock);
#endif
}

void
call_every_start(call_every_t *e)
{
    set_state(e, RUN);
}

void
call_every_stop(call_every_t *e)
{
    set_state(e, PAUSE);
}

void
call_every_destroy(call_every_t *e)
{
    set_state(e, STOP);
#ifndef PI_PICO
    pthread_join(e->thread, NULL);
    pthread_mutex_destroy(&e->lock);
    pthread_cond_destroy(&e->cond);
#endif
    free(e);
}
