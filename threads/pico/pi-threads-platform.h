#ifndef __PI_THREADS_PLATFORM_H__
#define __PI_THREADS_PLATFORM_H__

#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus

#include <list>
#include "semphr.h"

class PiThread {
public:
    PiThread(const char *name = "");
    virtual ~PiThread();

    PiThread *start();

    virtual void main() = 0;
    static void thread_entry(void *vp) {
	PiThread *t = (PiThread *) vp;
	t->main();
	vTaskDelete(NULL);
	delete t;
    }

private:
    const char *name;
    TaskHandle_t task;
};

class PiMutex {
public:
    PiMutex();
    ~PiMutex();
    void lock();
    int trylock();
    void unlock();

private:
    SemaphoreHandle_t m;
};

class PiCond {
public:
    PiCond();
    ~PiCond();
    int timedwait(PiMutex *m, const struct timespec *abstime);
    void wait(PiMutex *m);
    void signal();
    void broadcast();

private:
    PiMutex *lock;
    std::list<SemaphoreHandle_t> wait_list;
    SemaphoreHandle_t add_to_wait_list();
};

extern "C" {
#endif

void file_init();

#ifdef __cplusplus
};
#endif

#endif
