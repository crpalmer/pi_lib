#include <stdbool.h>
#include <assert.h>
#include "pi.h"
#include "mem.h"
#include "pi-threads.h"
#include "stop.h"

struct stopS {
    volatile bool requested;
    volatile bool stopped;
    pi_mutex_t *lock;
    pi_cond_t *cond;
};

stop_t *
stop_new(void)
{
    stop_t *stop = fatal_malloc(sizeof(*stop));

    stop->lock = pi_mutex_new();
    stop->cond = pi_cond_new();

    stop_reset(stop);

    return stop;
}

void
stop_request_stop(stop_t *stop)
{
    pi_mutex_lock(stop->lock);
    stop->requested = true;
    pi_mutex_unlock(stop->lock);
}

void
stop_await_stop(stop_t *stop)
{
    pi_mutex_lock(stop->lock);
    while (! stop->stopped) {
	pi_cond_wait(stop->cond, &stop->lock);
    }
    pi_mutex_unlock(stop->lock);
}

void
stop_stop(stop_t *stop)
{
    stop_request_stop(stop);
    stop_await_stop(stop);
}

bool
stop_requested(stop_t *stop)
{
    if (! stop) return false;
    return stop->requested;
}

void
stop_stopped(stop_t *stop)
{
    if (stop) {
	stop->stopped = true;
	pi_cond_signal(stop->cond);
    }
}

void
stop_reset(stop_t *stop)
{
    stop->requested = false;
    stop->stopped = false;
}

void
stop_destroy(stop_t *stop)
{
    pi_mutex_destroy(stop->lock);
    pi_cond_destroy(stop->cond);
    fatal_free(stop);
}

bool
stop_is_stopped(stop_t *stop)
{
    return stop->stopped;
}

