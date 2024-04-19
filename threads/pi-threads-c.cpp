#include "pi-threads.h"

class PiThreadWrapper : public PiThread {
public:
    PiThreadWrapper(const char *name, void (*main_fn)(void *), void *arg) : main_fn(main_fn), arg(arg), PiThread(name) { }

    void main(void) {
	main_fn(arg);
    }

private:
    void (*main_fn)(void *);
    void *arg;
};
	
void pi_thread_create(const char *name, void (*main)(void *), void *arg)
{
    new PiThreadWrapper(name, main, arg);
}

pi_mutex_t *pi_mutex_new()
{
    return new PiMutex();
}

void pi_mutex_lock(pi_mutex_t *m_vp)
{
    PiMutex *m = (PiMutex *) m_vp;
    m->lock();
}

int pi_mutex_trylock(pi_mutex_t *m_vp)
{
    PiMutex *m = (PiMutex *) m_vp;
    return m->trylock();
}

void pi_mutex_unlock(pi_mutex_t *m_vp)
{
    PiMutex *m = (PiMutex *) m_vp;
    m->unlock();
}

void pi_mutex_destroy(pi_mutex_t *m_vp)
{
}

pi_cond_t *pi_cond_new()
{
    return new PiCond();
}

int pi_cond_timedwait(pi_cond_t *c_vp, pi_mutex_t *m_vp, const struct timespec *abstime)
{
    PiCond *c = (PiCond *) c_vp;
    PiMutex *m = (PiMutex *) m_vp;

    return c->timedwait(m, abstime);
}

void pi_cond_wait(pi_cond_t *c_vp, pi_mutex_t *m_vp)
{
    PiCond *c = (PiCond *) c_vp;
    PiMutex *m = (PiMutex *) m_vp;

    c->wait(m);
}

void pi_cond_signal(pi_cond_t *c_vp)
{
    PiCond *c = (PiCond *) c_vp;
    c->signal();
}

void pi_cond_broadcast(pi_cond_t *c_vp)
{
    PiCond *c = (PiCond *) c_vp;
    c->broadcast();
}

void pi_cond_destroy(pi_cond_t *c_vp)
{
}
