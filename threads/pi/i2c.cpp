#include "pi.h"
#include "i2c.h"
#include "threaded-i2c.h"

#define MAX_BUS 10
static PiMutex *m[MAX_BUS];
static PiMutex *lock;

static void i2c_lock_impl(int bus) {
    if (m[bus] == NULL) {
        lock->lock();
	if (m[bus] == NULL) m[bus] = new PiMutex();
	lock->unlock();
    }
    m[bus]->lock();
}

static void i2c_lock_unlock(int bus) {
    m[bus]->unlock();
}

void i2c_threaded_init() {
    lock = new PiMutex();
    i2c_lock_fn = i2c_lock_impl;
    i2c_unlock_fn = i2c_unlock_impl;
}
