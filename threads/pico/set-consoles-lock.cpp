#include "pi-threads.h"
#include "consoles.h"
#include "pi-threads.h"
#include "set-consoles-lock.h"

class ConsolesMutexLock : public ConsolesLock {
public:
    ConsolesMutexLock() { m = new PiMutex(); }
    void lock() { m->lock(); }
    void unlock() {m->unlock(); }
private:
    PiMutex *m;
};

void set_consoles_lock() {
    consoles_set_consoles_lock(new ConsolesMutexLock());
}

