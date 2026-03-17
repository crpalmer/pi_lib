#include "pi.h"
#include "stepper.h"
#include "time-utils.h"

#include <math.h>

void Stepper::main(void) {
    us_time_t next_step = us_now() + 5;

    dir->set(1);
    while (1) {
	// If slowing down to a stop and we're in the jerk range then stop
	if (! target_v && -jerk <= v && v <= jerk) v = 0;

	// If not moving and not requested to move, go to sleep until we start moving
	if (! v && ! target_v) {
	    lock->lock();
	    enable->set(1);
	    while (! target_v) {
		cond->wait(lock);
	    }
	    enable->set(0);
	    lock->unlock();
	    next_step = us_now() + 5;	    // +5: don't lose time on the first step
	}

	// To avoid locking but keep all our computations consistent, grab the current
	// target_v and use it to compute the next step.  If the target velocity changes,
	// we'll deal with that on the next step.
	double cur_target_v = target_v;

	// If speeding up and we're in the jerk range, jump to the jerk speed
	if (cur_target_v != 0 && -jerk <= v && v <= jerk) v = cur_target_v > 0 ? jerk : -jerk;

	// Perform the step and get the delta time until the next step
	us_time_t next_step_delta = one_step(next_step);

	// Use the current time in case we aren't able to keep up with the step frequency.
	// If we just add to next_step and the requested step rate is faster than what we
	// can handle then we'll just end up falling further and further behind.
	//
	// This ensures we can't fall behind and the only risk is that we will be 1us slow
	// on the next step.
	next_step = us_now() + next_step_delta;

	// If we aren't at the target spped, apply acceleration
	if ((cur_target_v >= 0 && v < cur_target_v) ||
	    (cur_target_v <= 0 && v > cur_target_v)) {
	    double delta_v = acceleration * next_step_delta/1000000.0;
	    double new_v = v + (cur_target_v > 0 ? delta_v : -delta_v);

	    // Clamp jerk <= |new_v| <= |target_v|
	    if (cur_target_v >= 0 && new_v > cur_target_v) new_v = cur_target_v;
	    else if (cur_target_v <= 0 && new_v < cur_target_v) new_v = cur_target_v;
	    else if (-jerk < new_v && new_v < jerk) new_v = cur_target_v > 0 ? jerk : -jerk;

	    dir->set(new_v >= 0);
	    v = new_v;
        }
    }
}

us_time_t Stepper::one_step(us_time_t next_step) {
    if (v == 0) return 0;

    us_sleep_until(next_step);
    step->set(1);
    us_sleep(3);
    step->set(0);

    if (v > 0) n_steps++;
    else n_steps--;

    double distance = 1.0/steps_per_mm;
    double sec = distance / (v > 0 ? v : -v);
    return (us_time_t) (sec * 1000000);
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

double Stepper::get_mm_moved() {
    return n_steps / ((double) steps_per_mm);
}

void Stepper::reset_mm_moved() {
    n_steps = 0;
}

void Stepper::dump_state() {
    printf("%s: %.2f mm/sec (target %.2f), %.2f mm", name, v, target_v, get_mm_moved());
}

