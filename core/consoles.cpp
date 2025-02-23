#include <stdio.h>
#include <list>
#include <stdarg.h>
#include "consoles.h"
#include "core-lock.h"
#include "pi.h"
#include "time-utils.h"

class Consoles : public Writer {
public:
    static Consoles& get() {
        static Consoles instance;
        return instance;
    }

    void add(Console *c) {
	core_lock_lock(lock);
        active.push_back(c);
	core_lock_unlock(lock);
    }

    void remove(Console *c) {
	core_lock_lock(lock);
	active.remove(c);
	core_lock_unlock(lock);
    }

    int write_str(const char *str) override {
	core_lock_lock(lock);
	for (Console *c : active) c->write_str(str);
	if (active.empty()) fwrite(str, 1, strlen(str), stderr);
	core_lock_unlock(lock);
	return 0;
    }

private:
   std::list<Console *> active;
   core_lock_t lock = core_lock_create();
};

void consoles_add(Console *c) {
    Consoles::get().add(c);
}

void consoles_remove(Console *c) {
    Consoles::get().remove(c);
}

int consoles_write_str(const char *str) {
    return Consoles::get().write_str(str);
}

int consoles_printf(const char *fmt, ...) {
   va_list va;
   va_start(va, fmt);
   int ret = Consoles::get().printf_va(fmt, va);
   va_end(va);

   return ret;
}

void consoles_fatal_printf(const char *fmt, ...) {
   va_list va;
   va_start(va, fmt);
   Consoles::get().printf_va(fmt, va);
   va_end(va);

   fflush(stderr);	/* In case */
   ms_sleep(5000);

   pi_abort();
}
