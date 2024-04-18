#include "pi-threads.h"
#include "consoles.h"
#include "pi-threads.h"
#include "set-consoles-lock.h"

class ConsolesMutexLock : public ConsolesLock {
public:
    ConsolesMutexLock() { m = pi_mutex_new(); }
    void lock() { pi_mutex_lock(m); }
    void unlock() { pi_mutex_unlock(m); }
private:
    pi_mutex_t *m;
};

void set_consoles_lock() {
    consoles_set_consoles_lock(new ConsolesMutexLock());
}

