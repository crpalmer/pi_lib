#include <stdio.h>
#include <pthread.h>
#include "pi.h"
#include "mem.h"
#include "time-utils.h"

#include "pi-threads.h"

struct pi_mutexS {
   pthread_mutex_t m;
};

struct pi_condS {
   pthread_cond_t c;
};

void pi_init_with_threads(void)
{
    pi_init();
}

void pi_threads_start_and_wait()
{
    while (1) ms_sleep(10000);
}

void pi_thread_create(const char *name, void (*main)(void *), void *arg)
{
    pthread_t t;

    if (pthread_create(&t, NULL, (void *(*)(void *)) main, arg)) {
        pthread_detach(t);
    }
}

pi_mutex_t *pi_mutex_new()
{
    pi_mutex_t *m = fatal_malloc(sizeof(*m));
    pthread_mutex_init(&m->m, NULL);
    return m;
}

void pi_mutex_lock(pi_mutex_t *m)
{
    pthread_mutex_lock(&m->m);
}

int pi_mutex_trylock(pi_mutex_t *m)
{
    return pthread_mutex_trylock(&m->m);
}

void pi_mutex_unlock(pi_mutex_t *m)
{
    pthread_mutex_unlock(&m->m);
}

void pi_mutex_destroy(pi_mutex_t *m)
{
    free(m);
}

pi_cond_t *pi_cond_new()
{
    pthread_condattr_t attr;
    pi_cond_t *c = fatal_malloc(sizeof(*c));

    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);

    pthread_cond_init(&c->c, &attr);
    return c;
}

int pi_cond_timedwait(pi_cond_t *c, pi_mutex_t *m, const struct timespec *abstime)
{
    return pthread_cond_timedwait(&c->c, &m->m, abstime);
}

void pi_cond_wait(pi_cond_t *c, pi_mutex_t *m)
{
    pthread_cond_wait(&c->c, &m->m);
}

void pi_cond_signal(pi_cond_t *c)
{
    pthread_cond_signal(&c->c);
}

void pi_cond_broadcast(pi_cond_t *c)
{
    pthread_cond_broadcast(&c->c);
}

void pi_cond_destroy(pi_cond_t *c)
{
    free(c);
}


