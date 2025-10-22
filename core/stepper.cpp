#include <math.h>
#include "pi.h"
#include "stepper.h"

void Stepper::go(double target, double feed) {
    if (feed <= 0) feed = last_feed;
    else last_feed = feed;

    double us_to_sleep = (1000*1000.0) / (feed * steps_per_mm); 
    unsigned delay = (unsigned) us_to_sleep;
    int n_steps = fabs(target - pos) * steps_per_mm;
    bool direction = (target > pos);

    printf("go from %lf to %lf @ %lf\n", pos, target, last_feed);
    printf("gen %d steps in direction %d with %f (%u) delay\n", n_steps, direction, us_to_sleep, delay);

    dir->set(direction);

    for (int i = 0; i < n_steps; i++) {
	step->set(1);
	us_sleep(5);
	step->set(0);
	if (delay > 5) us_sleep(delay - 5);
    }
	
    if (direction) pos += n_steps / steps_per_mm;
    else pos -= n_steps / steps_per_mm;
}

void Stepper::home(Input *end_stop, double homed_pos, double feed, double mm_per_check) {
    while (! end_stop->get()) {
	go(pos + mm_per_check, feed);
    }
    pos = homed_pos;
}

void set_acceleration(double mm_per_sec_squared) {}
void set_jerk(double mm_per_sec) {}
