#ifndef __PI_THREADS_PLATFORM_H__
#define __PI_THREADS_PLATFORM_H__

#ifdef __cplusplus

#include <pthread.h>

class PiThread {
public:
    PiThread(const char *name = "");
    ~PiThread();

    PiThread *start(int priority = 1, int affinity = -1);

    virtual void main() = 0;
    static void thread_entry(void *vp);

    const char *get_name() { return name; }

    void yield() { }
    void pause();
    void resume();
    void resume_from_isr() { resume(); }

    static PiThread *self();

private:
    pthread_t t;
    const char *name;
    class PiMutex *m_paused;
    class PiCond *c_paused;
    bool resumed = false;
};

class PiMutex {
public:
    PiMutex();
    void lock();
    bool trylock();
    void unlock();

private:
    pthread_mutex_t m;

    friend class PiCond;
};

class PiCond {
public:
    PiCond();
    bool wait(PiMutex *m, const struct timespec *abstime = NULL);
    void signal();
    void broadcast();

private:
    pthread_cond_t c;
};

#endif

#endif
