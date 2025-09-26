#include <math.h>
#include "pi.h"
#include "stepper.h"

Stepper::Stepper(int base_pin, double steps_per_mm, const char *name) : PiThread(name), steps_per_mm(steps_per_mm), mm_per_step(1.0 / steps_per_mm) {
    lock = new PiMutex();
    cond = new PiCond();
    device = picostepper_init(base_pin, TwoWireDriver);
    picostepper_set_async_enabled(device, true);
    start(5);
}

static void move_finished(PicoStepper device, void *user_data) {
    Stepper *stepper = (Stepper *) user_data;
    stepper->resume_from_isr();
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
	double us_to_sleep = (1000*1000.0) / (last_feed * steps_per_mm); 
	unsigned delay = (unsigned) (us_to_sleep + 0.5);
	lock->unlock();
	int n_steps = fabs(target - pos) * steps_per_mm;
	bool direction = target > pos;
	printf("gen %d steps in direction %d with %f (%u) delay\n", n_steps, direction, us_to_sleep, delay);
	picostepper_set_async_direction(device, direction);
	picostepper_set_async_delay(device, delay);
	picostepper_move_async(device, n_steps, move_finished, this);
	pause();
	lock->lock();
	if (direction) pos += n_steps / steps_per_mm;
	else pos -= n_steps / steps_per_mm;
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
