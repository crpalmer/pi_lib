#include <stdio.h>
#include <list>
#include <stdarg.h>
#include "consoles.h"
#include "consoles-lock.h"
#include "pi.h"

class Consoles : public Console, OnDeath<Console *> {
public:
    static Consoles& get() {
        static Consoles instance;
        return instance;
    }

    void set_consoles_lock(ConsolesLock *l) {
        lock = l;
    }

    void add(Console *c) {
	lock->lock();
        active.push_back(c);
	lock->unlock();
	c->set_on_death_notifier(this);
    }

    void on_death(Console *c) {
	lock->lock();
	active.remove(c);
	lock->unlock();
    }

    int write_str(const char *str) override {
	lock->lock();
	for (Console *c : active) c->write_str(str);
	if (active.empty()) fwrite(str, 1, strlen(str), stderr);
	lock->unlock();
	return 0;
    }

private:
   std::list<Console *> active;
   ConsolesLock *lock = new ConsolesLock();
};

void consoles_add(Console *c) {
    Consoles::get().add(c);
}

void consoles_on_death(Console *c) {
    Consoles::get().on_death(c);
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

   pi_reboot_bootloader();
}

void consoles_set_consoles_lock(ConsolesLock *l) {
   Consoles::get().set_consoles_lock(l);
}
