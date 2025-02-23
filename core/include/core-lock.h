#ifndef __CORE_LOCKS_H__
#define __CORE_LOCKS_H__

typedef void *core_lock_t;

typedef core_lock_t (*core_lock_create_fn)();
typedef void (*core_lock_lock_fn)(core_lock_t);
typedef void (*core_lock_unlock_fn)(core_lock_t);
typedef void (*core_lock_destroy_fn)(core_lock_t);

core_lock_t core_lock_create();
void core_lock_lock(core_lock_t);
void core_lock_unlock(core_lock_t);
void core_lock_destroy(core_lock_t);

void core_lock_init(core_lock_create_fn create_fn, core_lock_lock_fn lock_fn, core_lock_unlock_fn unlock_fn, core_lock_destroy_fn destroy_fn);

#endif
