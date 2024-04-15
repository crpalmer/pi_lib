#ifndef __PI_THREADS_H__
#define __PI_THREADS_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PLATFORM_pico
#include "FreeRTOS.h"
#include "task.h"
#endif

typedef struct pi_threadS pi_thread_t;
typedef struct pi_mutexS pi_mutex_t;
typedef struct pi_condS pi_cond_t;

void pi_threads_init(void);
void pi_threads_start_and_wait();
void pi_threads_dump_state();

pi_thread_t *pi_thread_new(void (*thread_main)(void *arg), void *thread_arg);
void pi_thread_create_anonymous(void (*thread_main)(void *arg), void *thread_arg);

pi_mutex_t *pi_mutex_new();
void pi_mutex_lock(pi_mutex_t *);
int pi_mutex_trylock(pi_mutex_t *);
void pi_mutex_unlock(pi_mutex_t *);
void pi_mutex_destroy(pi_mutex_t *);

pi_cond_t *pi_cond_new();
int pi_cond_timedwait(pi_cond_t *cond, pi_mutex_t *mutex, const struct timespec *abstime);
void pi_cond_wait(pi_cond_t *cond, pi_mutex_t *mutex);
void pi_cond_signal(pi_cond_t *cond);
void pi_cond_destroy(pi_cond_t *cond);

#ifdef __cplusplus
};
#endif

#endif
