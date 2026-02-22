#include "pi.h"
#include <pthread.h>
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
    pthread_t          thread;
    pthread_mutex_t    lock;
    pthread_cond_t     cond;
};

static void *
thread_main(void *e_as_vp)
{
    call_every_t *e = (call_every_t *) e_as_vp;
    nano_time_t last;

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
		nano_time_t now;
		nano_gettime(&now);
		fprintf(stderr, "ce at: %ld want %ld\n", now, last);
	    }
	    e->func(e->data);
	    pthread_mutex_lock(&e->lock);
	}
    }

    return NULL;
}

call_every_t *
call_every_new(unsigned ms, call_every_func_t func, void *data)
{
    call_every_t *e;

    assert(func != NULL);

    e = fatal_malloc(sizeof(*e));
    e->ms   = ms;
    e->func = func;
    e->data = data;

    pthread_mutex_init(&e->lock, NULL);
    pthread_cond_init(&e->cond, NULL);

    pthread_create(&e->thread, NULL, thread_main, e);

    return e;
}

static void
set_state(call_every_t *e, call_every_state_t state)
{
    assert(e);

    pthread_mutex_lock(&e->lock);
    e->state = state;
    pthread_cond_signal(&e->cond);
    pthread_mutex_unlock(&e->lock);
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
    pthread_join(e->thread, NULL);
    pthread_mutex_destroy(&e->lock);
    pthread_cond_destroy(&e->cond);
    fatal_free(e);
}
