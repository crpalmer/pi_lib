#ifndef __PI_THREADS_PLATFORM_H__
#define __PI_THREADS_PLATFORM_H__

#ifdef __cplusplus

#include <pthread.h>

class PiThread {
public:
    PiThread(const char *name = "");
    ~PiThread();

    virtual void main() = 0;
    static void thread_entry(void *vp) {
	PiThread *t = (PiThread *) vp;
	t->main();
    }

private:
    pthread_t t;
    const char *name;
};

class PiMutex {
public:
    PiMutex();
    void lock();
    int trylock();
    void unlock();

private:
    pthread_mutex_t m;

    friend class PiCond;
};

class PiCond {
public:
    PiCond();
    int timedwait(PiMutex *m, const struct timespec *abstime);
    void wait(PiMutex *m);
    void signal();
    void broadcast();

private:
    pthread_cond_t c;
};

#endif

#endif
