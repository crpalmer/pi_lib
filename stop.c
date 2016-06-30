#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include "mem.h"
#include "stop.h"

struct stopS {
    volatile bool requested;
    volatile bool stopped;
    pthread_mutex_t lock;
    pthread_cond_t cond;
};

stop_t *
stop_new(void)
{
    stop_t *stop = fatal_malloc(sizeof(*stop));

    pthread_mutex_init(&stop->lock, NULL);
    pthread_cond_init(&stop->cond, NULL);

    stop_reset(stop);

    return stop;
}

void
stop_request_stop(stop_t *stop)
{
    pthread_mutex_lock(&stop->lock);
    assert(! stop->requested);
    stop->requested = true;
    pthread_mutex_unlock(&stop->lock);
}

void
stop_await_stop(stop_t *stop)
{
    pthread_mutex_lock(&stop->lock);
    while (! stop->stopped) {
	pthread_cond_wait(&stop->cond, &stop->lock);
    }
    pthread_mutex_unlock(&stop->lock);
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
	pthread_cond_signal(&stop->cond);
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
    free(stop);
}
