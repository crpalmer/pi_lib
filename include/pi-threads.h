#ifndef __PI_THREADS_H__
#define __PI_THREADS_H__

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void pi_mutex_t;
typedef void pi_cond_t;

typedef void (*pi_threads_main_t)(int argc, char **argv);

/* Note: argc/argv are ignored in the pico implementation as
 * there is no commandline for a pico.
 */
void pi_init_with_threads(pi_threads_main_t main, int argc, char **argv);

size_t pi_threads_get_free_ram();
void pi_threads_dump_state();

void pi_thread_create(const char *name, void (*thread_main)(void *arg), void *thread_arg);

pi_mutex_t *pi_mutex_new();
void pi_mutex_lock(pi_mutex_t *);
bool pi_mutex_trylock(pi_mutex_t *);
void pi_mutex_unlock(pi_mutex_t *);
void pi_mutex_destroy(pi_mutex_t *);

pi_cond_t *pi_cond_new();
int pi_cond_timedwait(pi_cond_t *cond, pi_mutex_t *mutex, const struct timespec *abstime);
void pi_cond_wait(pi_cond_t *cond, pi_mutex_t *mutex);
void pi_cond_signal(pi_cond_t *cond);
void pi_cond_broadcast(pi_cond_t *cond);
void pi_cond_destroy(pi_cond_t *cond);

#ifdef __cplusplus

};

#include <string>
std::string pi_threads_get_state();

#endif

#include "pi-threads-platform.h"

#endif
