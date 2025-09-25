#include <math.h>
#include "pi.h"
#include "stepper.h"

Stepper::Stepper(Output *dir, Output *step, double steps_per_mm, const char *name) : PiThread(name), dir(dir), step(step), steps_per_mm(steps_per_mm), mm_per_step(1.0 / steps_per_mm) {
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
	    cond->wait(lock);
	}
	double us_to_sleep = (1000*1000.0) / (last_feed * steps_per_mm) / 2.0; 
	lock->unlock();
	dir->set(target > pos);
	while (fabs(target - pos) > mm_per_step) {
	    step->on();
	    ms_sleep((us_to_sleep+500)/1000);
	    step->off();
	    if (pos < target) pos += mm_per_step;
	    else pos -= mm_per_step;
	    ms_sleep(us_to_sleep/1000);
	}
	lock->lock();
	active = false;
	cond->signal();
    }
}

void Stepper::go(double dest, double feed, bool async) {
    lock->lock();
    if (feed == 0) feed = last_feed;
    else last_feed = feed;
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
