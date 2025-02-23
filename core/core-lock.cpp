#include "pi.h"
#include "core-lock.h"

static core_lock_create_fn create_fn = NULL;
static core_lock_lock_fn lock_fn = NULL;
static core_lock_unlock_fn unlock_fn = NULL;
static core_lock_destroy_fn destroy_fn = NULL;

void core_lock_init(core_lock_create_fn _create_fn, core_lock_lock_fn _lock_fn, core_lock_unlock_fn _unlock_fn, core_lock_destroy_fn _destroy_fn) {
    create_fn = _create_fn;
    lock_fn = _lock_fn;
    unlock_fn = _unlock_fn;
    destroy_fn = _destroy_fn;
}

core_lock_t core_lock_create() {
    if (create_fn) return create_fn();
    return NULL;
}

void core_lock_lock(core_lock_t lock) {
    if (lock && lock_fn) lock_fn(lock);
}

void core_lock_unlock(core_lock_t lock) {
    if (lock && unlock_fn) unlock_fn(lock);
}

void core_lock_destroy(core_lock_t lock) {
    if (lock && destroy_fn) destroy_fn(lock);
}
