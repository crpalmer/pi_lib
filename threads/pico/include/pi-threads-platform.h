#ifndef __PI_THREADS_PLATFORM_H__
#define __PI_THREADS_PLATFORM_H__

#ifdef __cplusplus

#include <list>

/* Notify indices:

   #0 is used internally by the sdk (?)
   #1 is potentially used by the fat code (sdio)
   #2 is potentially used by the fat code (spi)
 */

#define PI_THREAD_NOTIFY_INDEX 3

/* Thread local storage */

#define PI_THREAD_LOCAL_PI_THREAD 0

typedef void *task_handle_t;
typedef void *semaphore_handle_t;

class PiThread {
public:
    PiThread(const char *name = "<unnamed>");
    virtual ~PiThread();

    PiThread *start(int priority = 1, int affinity = -1);
    static void thread_entry(void *vp);
    virtual void main() = 0;

    void pause();
    void resume();
    void resume_from_isr();

    static PiThread *self();

private:
    const char *name;
    task_handle_t task;
};

class PiMutex {
public:
    PiMutex();
    ~PiMutex();
    void lock();
    bool trylock();
    void unlock();

private:
    semaphore_handle_t m;
};

class PiCond {
public:
    PiCond();
    ~PiCond();
    bool wait(PiMutex *m, const struct timespec *abstime = NULL);
    void signal();
    void broadcast();

private:
    PiMutex *lock;
    std::list<task_handle_t> wait_list;
    void wake_one_locked();
};

extern "C" {
#endif

void file_init();

#ifdef __cplusplus
};
#endif

#endif
