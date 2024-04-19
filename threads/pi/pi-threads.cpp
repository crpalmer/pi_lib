#include <stdio.h>
#include <stdlib.h>
#include <list>
#include "pi.h"
#include "time-utils.h"

#include "pi-threads.h"

static pthread_mutex_t at_lock = PTHREAD_MUTEX_INITIALIZER;
static std::list<PiThread *> active_threads;

void pi_init_with_threads(void)
{
    pi_init();
}

void pi_threads_start_and_wait()
{
    while (1) ms_sleep(10000);
}

PiThread::PiThread(const char *name) : name(name) {
    if (pthread_create(&t, NULL, (void *(*)(void *)) PiThread::thread_entry, this)) {
        pthread_detach(t);

	pthread_mutex_lock(&at_lock);
	active_threads.push_back(this);
	pthread_mutex_unlock(&at_lock);
    }
}

PiThread::~PiThread() {
    pthread_mutex_lock(&at_lock);
    active_threads.remove(this);
    if (active_threads.empty()) {
	fprintf(stderr, "All threads have exited.\n");
	exit(0);
    }
    pthread_mutex_unlock(&at_lock);
}
    
PiMutex::PiMutex() {
    pthread_mutex_init(&m, NULL);
}

void PiMutex::lock() {
    pthread_mutex_lock(&m);
}

int PiMutex::trylock() {
    return pthread_mutex_trylock(&m);
}

void PiMutex::unlock() {
    pthread_mutex_unlock(&m);
}

PiCond::PiCond() {
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&c, &attr);
}

int PiCond::timedwait(PiMutex *m, const struct timespec *abstime) {
    return pthread_cond_timedwait(&c, &m->m, abstime);
}

void PiCond::wait(PiMutex *m) {
    pthread_cond_wait(&c, &m->m);
}

void PiCond::signal() {
    pthread_cond_signal(&c);
}

void PiCond::broadcast() {
    pthread_cond_broadcast(&c);
}
