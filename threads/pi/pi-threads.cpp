#include <stdio.h>
#include <stdlib.h>
#include <list>
#include "pi.h"
#include "consoles.h"
#include "time-utils.h"

#include "pi-threads.h"

static pthread_mutex_t at_lock = PTHREAD_MUTEX_INITIALIZER;
static std::list<PiThread *> active_threads;

void pi_init_with_threads(pi_threads_main_t main, int argc, char **argv) {
    pi_init();
    main(argc, argv);
}

PiThread::PiThread(const char *name) : name(name) {
}

PiThread *PiThread::start(int priority_unused) {
    if (pthread_create(&t, NULL, (void *(*)(void *)) PiThread::thread_entry, this)) {
        pthread_detach(t);

	pthread_mutex_lock(&at_lock);
	active_threads.push_back(this);
	pthread_mutex_unlock(&at_lock);
    }
    return this;
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

size_t pi_threads_get_free_ram() {
    return 0;  // TODO: parse /proc/meminfo and report it
}

void pi_threads_dump_state() {
    consoles_printf("No state dump available for the Pi\n");
}
