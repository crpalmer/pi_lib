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
    repeating_timer_t  timer;
};

static bool
callback(repeating_timer_t *t)
{
    call_every_t *e = (call_every_t *) t->user_data;
    e->func(e->data);
    return true;
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

    return e;
}

static void
set_state(call_every_t *e, call_every_state_t state) {
    if (state == RUN) {
       alarm_pool_add_repeating_timer_ms(alarm_pool_get_default(), e->ms, callback, e, &e->timer);
    } else {
       cancel_repeating_timer(&e->timer);
    }
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
    free(e);
}
