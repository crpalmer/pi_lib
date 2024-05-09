#ifndef __PI_THREADS_PLATFORM_H__
#define __PI_THREADS_PLATFORM_H__

#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus

#include <list>
#include "semphr.h"

class PiThread {
public:
    PiThread(const char *name = "<unnamed>");
    virtual ~PiThread();

    PiThread *start(int priority = 1);

    virtual void main() = 0;

    static void thread_entry(void *vp) {
	PiThread *t = (PiThread *) vp;
	t->main();
	vTaskDelete(NULL);
	delete t;
    }

    void pause();
    void resume();
    void resume_from_isr();

private:
    const char *name;
    TaskHandle_t task;
};

class PiMutex {
public:
    PiMutex();
    ~PiMutex();
    void lock();
    bool trylock();
    void unlock();

private:
    SemaphoreHandle_t m;
};

class PiCond {
public:
    PiCond();
    ~PiCond();
    bool timedwait(PiMutex *m, const struct timespec *abstime);
    void wait(PiMutex *m);
    void signal();
    void broadcast();

private:
    PiMutex *lock;
    std::list<TaskHandle_t> wait_list;
    void wake_one_locked();
};

extern "C" {
#endif

void file_init();

#ifdef __cplusplus
};
#endif

#endif
