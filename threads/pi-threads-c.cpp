#include "pi.h"
#include "core-lock.h"
#include "pi-threads.h"

#define C_DECL extern "C"

core_lock_t core_lock_create_impl() { return pi_mutex_new(); }
void core_lock_lock_impl(core_lock_t lock) { pi_mutex_lock((pi_mutex_t *) lock); }
void core_lock_unlock_impl(core_lock_t lock) { pi_mutex_unlock((pi_mutex_t *) lock); }
void core_lock_destroy_impl(core_lock_t lock) { pi_mutex_destroy((pi_mutex_t *) lock); }

void pi_init_with_threads(pi_threads_main_t main, int argc, char **argv) {
    pi_init_no_reboot();
    core_lock_init(&core_lock_create_impl, &core_lock_lock_impl, &core_lock_unlock_impl, &core_lock_destroy_impl);
    platform_init_with_threads(main, argc, argv);
}

class PiThreadWrapper : public PiThread {
public:
    PiThreadWrapper(const char *name, void (*main_fn)(void *), void *arg) : PiThread(name), main_fn(main_fn), arg(arg) {
        start();
    }
    virtual ~PiThreadWrapper() {}

    void main(void) {
	main_fn(arg);
	delete this;
    }

private:
    void (*main_fn)(void *);
    void *arg;
};
	
C_DECL void pi_thread_create(const char *name, void (*main)(void *), void *arg)
{
    new PiThreadWrapper(name, main, arg);
}

C_DECL pi_mutex_t *pi_mutex_new()
{
    return new PiMutex();
}

C_DECL void pi_mutex_lock(pi_mutex_t *m_vp)
{
    PiMutex *m = (PiMutex *) m_vp;
    m->lock();
}

C_DECL bool pi_mutex_trylock(pi_mutex_t *m_vp)
{
    PiMutex *m = (PiMutex *) m_vp;
    return m->trylock();
}

C_DECL void pi_mutex_unlock(pi_mutex_t *m_vp)
{
    PiMutex *m = (PiMutex *) m_vp;
    m->unlock();
}

C_DECL void pi_mutex_destroy(pi_mutex_t *m_vp)
{
    delete (PiMutex *) m_vp;
}

C_DECL pi_cond_t *pi_cond_new()
{
    return new PiCond();
}

C_DECL int pi_cond_timedwait(pi_cond_t *c_vp, pi_mutex_t *m_vp, const nano_time_t *abstime)
{
    PiCond *c = (PiCond *) c_vp;
    PiMutex *m = (PiMutex *) m_vp;

    return c->wait(m, abstime);
}

C_DECL void pi_cond_wait(pi_cond_t *c_vp, pi_mutex_t *m_vp)
{
    PiCond *c = (PiCond *) c_vp;
    PiMutex *m = (PiMutex *) m_vp;

    c->wait(m);
}

C_DECL void pi_cond_signal(pi_cond_t *c_vp)
{
    PiCond *c = (PiCond *) c_vp;
    c->signal();
}

C_DECL void pi_cond_broadcast(pi_cond_t *c_vp)
{
    PiCond *c = (PiCond *) c_vp;
    c->broadcast();
}

C_DECL void pi_cond_destroy(pi_cond_t *c_vp)
{
    delete (PiCond *) c_vp;
}
