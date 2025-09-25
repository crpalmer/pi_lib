#include <math.h>
#include "pi.h"
#include "stepper.h"

Stepper::Stepper(Output *dir, Output *step, double steps_per_mm, const char *name) : PiThread(name), dir(dir), step(step), mm_per_step(1.0 / steps_per_mm) {
    lock = new PiMutex();
    cond = new PiCond();
    start(5);
}

Stepper::~Stepper() {
    delete cond;
    delete lock;
}

void Stepper::main() {
    lock->lock();
    while (true) {
	while (! active) {
	    printf("waiting\n");
	    cond->wait(lock);
	    printf("going\n");
	}
	lock->unlock();
	printf("dir %d\n", target > pos);
	dir->set(target > pos);
	while (fabs(target - pos) > mm_per_step) {
	    step->on();
	    ms_sleep(1);
	    step->off();
	    if (pos < target) pos += mm_per_step;
	    else pos -= mm_per_step;
	    ms_sleep(1);
	}
	lock->lock();
	printf("done\n");
	active = false;
	cond->signal();
    }
}

void Stepper::go(double dest, double feed, bool async) {
    lock->lock();
    printf("move to %f\n", dest);
    while (active) {
	cond->wait(lock);
    }
    active = true;
    target = dest;
    cond->signal();

async = false;

    if (! async) {
	while (active) {
	    cond->wait(lock);
	}
    }

    lock->unlock();
}

void set_endstop(Input *end_stop, double low, double high, bool is_low = true) {}
void set_acceleration(double mm_per_sec_squared) {}
void set_jerk(double mm_per_sec) {}
