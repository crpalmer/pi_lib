#include "pi.h"
#include "stepper.h"
#include "time-utils.h"

#include <math.h>

void Stepper::main(void) {
    us_time_t last_step = us_now();

    dir->set(1);
    while (1) {
	if (! target_v && jerk >= v && v >= -jerk) v = 0;

	if (! v && ! target_v) {
	    lock->lock();
	    enable->set(1);
	    while (! target_v) {
		cond->wait(lock);
	    }
	    enable->set(0);
	    lock->unlock();
	    last_step = us_now();
	}

	if (! v && target_v != 0) v = target_v > 0 ? jerk : -jerk;

	one_step();

	us_time_t now = us_now();
	us_time_t elapsed_us = now - last_step;
	last_step = now;

	double delta_v = acceleration * elapsed_us/1000000.0;
	double new_v;

	if (v > target_v) new_v = v - delta_v;
	else if (v < target_v) new_v = v + delta_v;
	else new_v = v;

	if (target_v > 0 && new_v > target_v) new_v = target_v;
	else if (target_v < 0 && new_v < target_v) new_v = target_v;
	else if (v <= -jerk && new_v >= -jerk && new_v <  jerk) new_v = jerk;
	else if (v >=  jerk && new_v <=  jerk && new_v > -jerk) new_v = -jerk;

	if (v != new_v) dir->set(new_v >= 0);

	v = new_v;
    }
}

void Stepper::one_step() {
    if (! v) return;

    double distance = 1.0/steps_per_mm;
    double sec = distance / (v > 0 ? v : -v);
    us_time_t us = (us_time_t) (sec * 1000000);

    step->set(1);
    us_sleep(3);
    step->set(0);
    if (us > 3) us_sleep(us - 3);
}
    
void Stepper::set_speed(double mm_per_sec) {
    lock->lock();
    target_v = mm_per_sec;
    cond->signal();
    lock->unlock();
}

void Stepper::set_acceleration(double mm_per_sec2) {
    acceleration = mm_per_sec2;
}

void Stepper::set_jerk(double mm_per_sec) {
    jerk = mm_per_sec;
}

void Stepper::set_steps_per_mm(double steps_per_mm) {
    this->steps_per_mm = steps_per_mm;
}
