#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <sys/prctl.h>
#include "pi.h"
#include "consoles.h"
#include "i2c.h"
#include "time-utils.h"

#include "pi-threads.h"

static pthread_mutex_t at_lock = PTHREAD_MUTEX_INITIALIZER;
static std::list<PiThread *> active_threads;
static pthread_key_t pi_thread_local;

void platform_init_with_threads(pi_threads_main_t main, int argc, char **argv) {
    pthread_key_create(&pi_thread_local, NULL);

    main(argc, argv);

    pthread_mutex_lock(&at_lock);
    if (active_threads.empty()) {
	fprintf(stderr, "All threads have exited.\n");
	exit(0);
    }
    pthread_mutex_unlock(&at_lock);
    while (1) { ms_sleep(10*1000); }
}

PiThread::PiThread(const char *name) : name(name) {
    pthread_mutex_lock(&at_lock);
    active_threads.push_back(this);
    pthread_mutex_unlock(&at_lock);
    m_paused = new PiMutex();
    c_paused = new PiCond();
}

PiThread *PiThread::start(int priority_unused, int affinity_unused) {
    if (pthread_create(&t, NULL, (void *(*)(void *)) PiThread::thread_entry, this)) {
        pthread_detach(t);
    }
    return this;
}

void PiThread::thread_entry(void *vp) {
    PiThread *t = (PiThread *) vp;
    prctl(PR_SET_NAME, t->get_name());
    pthread_setspecific(pi_thread_local, t);
    t->main();
}

PiThread *PiThread::self() {
    return (PiThread *) pthread_getspecific(pi_thread_local);
}

PiThread::~PiThread() {
    delete c_paused;
    delete m_paused;

    pthread_mutex_lock(&at_lock);
    active_threads.remove(this);
    if (active_threads.empty()) {
	fprintf(stderr, "All threads have exited.\n");
	exit(0);
    }
    pthread_mutex_unlock(&at_lock);
}

void PiThread::pause() {
    m_paused->lock();
    while (! resumed) {
	c_paused->wait(m_paused);
    }
    resumed = false;
    m_paused->unlock();
}

void PiThread::resume() {
    resumed = true;
    c_paused->signal();
}
    
PiMutex::PiMutex() {
    pthread_mutex_init(&m, NULL);
}

void PiMutex::lock() {
    pthread_mutex_lock(&m);
}

bool PiMutex::trylock() {
    return pthread_mutex_trylock(&m) >= 0;
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

bool PiCond::wait(PiMutex *m, const nano_time_t *abstime) {
    if (abstime) return pthread_cond_timedwait(&c, &m->m, abstime) == 0;
    pthread_cond_wait(&c, &m->m);
    return true;
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

std::string pi_threads_get_state() {
    return "No state dump available for the Pi\n";
}
