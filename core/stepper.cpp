#include <math.h>
#include "pi.h"
#include "stepper.h"

static const bool DEBUG = false;

class StepperClock : public PhysicsClock {
public:
    void start() override { now = 0; }
    double get() override { return now; }
    void inc(int us) { now += us / (1000.0*1000.0); }
    void set(double sec) { now = sec; }

private:
    double now = 0;
};

Stepper::Stepper(Output *dir, Output *step, double steps_per_mm) : steps_per_mm(steps_per_mm), mm_per_step(1.0 / steps_per_mm), dir(dir), step(step) {
    clock = new StepperClock();
    physics = new Physics(accel, last_feed - jerk, clock);
}

Stepper::~Stepper() {
    delete physics;
    delete clock;
}

static inline double delay_of_mm_per_sec(double mm_per_sec, double steps_per_mm) {
    return (1000*1000.0) / (mm_per_sec * steps_per_mm); 
}

static inline void one_step(Output *step, unsigned delay) {
    step->set(1);
    us_sleep(5);
    step->set(0);
    if (delay > 5) us_sleep(delay - 5);
}

static inline double get_v(Physics *physics, double jerk) {
    return physics->get_v() + jerk;
}

static inline unsigned physics_steps(StepperClock *clock, Physics *physics, Output *step, double steps_per_mm, int n_steps, double target_v, double jerk, Input *end_stop = NULL) {
    double v = 0;

    while (n_steps && (target_v <= 0 || (v = get_v(physics, jerk)) < target_v) && (target_v > 0 || (v = get_v(physics, jerk)) > jerk)) {
	double step_delay = delay_of_mm_per_sec(v, steps_per_mm);
	one_step(step, step_delay);
	clock->inc(step_delay);
	if (DEBUG && n_steps % 10 == 0) printf("target_v %f jerk %f v %f n_steps %d delay %f -> t %f\n", target_v, jerk, v, n_steps, step_delay, clock->get());
	n_steps--;
	if (end_stop && end_stop->get()) return n_steps;
    }

    if (DEBUG) printf("%d steps left final_v %f\n", n_steps, v);
    return n_steps;
}

void Stepper::go(double target, double feed) {
    StepperClock *clk = (StepperClock *) clock;

    if (feed <= 0) {
	feed = last_feed;
    } else {
	last_feed = feed;
	physics->set_max_velocity((feed > jerk) ? (feed - jerk) : jerk);
    }

    if (fabs(target - pos) < 1.0 / steps_per_mm) return;

    double delay = delay_of_mm_per_sec(feed, steps_per_mm);
    int n_steps = fabs(target - pos) * steps_per_mm;
    int starting_n_steps = n_steps;
    bool direction = (target > pos);

    dir->set(direction);
    physics->start_motion(0, fabs(target - pos));

    double decel_t = physics->get_deceleration_t();

    if (DEBUG) printf("go from %lf to %lf @ %lf\n", pos, target, last_feed);
    if (DEBUG) printf("gen %d steps in direction %d with %f us delay and %f accel (decel @ %f)\n", n_steps, direction, delay, accel, decel_t);

    n_steps = physics_steps(clk, physics, step, steps_per_mm, n_steps, feed, jerk);

    unsigned accel_steps = starting_n_steps - n_steps;
    int n_full_speed_steps = starting_n_steps - 2*accel_steps;

    if (n_full_speed_steps > 0) {
	for (int i = 0; i < n_full_speed_steps; i++) {
	    one_step(step, delay);
	}

	n_steps -= n_full_speed_steps;
	clk->set(decel_t);
	if (DEBUG) printf("%d accel steps -> %d full speed steps -> n_steps %d @ %f\n", accel_steps, n_full_speed_steps, n_steps, clk->get());
    }

    n_steps = physics_steps(clk, physics, step, steps_per_mm, n_steps, 0, jerk);

    n_steps = starting_n_steps - n_steps;
    if (direction) pos += n_steps / steps_per_mm;
    else pos -= n_steps / steps_per_mm;

}

void Stepper::home(Input *end_stop, bool direction, double homed_pos, double feed) {
    last_feed = feed;
    physics->set_max_velocity((feed > jerk) ? (feed - jerk) : jerk);
    physics->start_motion(0, 10*1000);

    dir->set(direction);
    physics_steps((StepperClock *) clock, physics, step, steps_per_mm, 1000*1000, feed, jerk);

    double delay = delay_of_mm_per_sec(feed, steps_per_mm);

    while (! end_stop->get()) {
	one_step(step, delay);
    }
    pos = homed_pos;
}

void Stepper::set_acceleration(double mm_per_sec_squared) {
    accel = mm_per_sec_squared;
    physics->set_acceleration(accel);
}

void Stepper::set_jerk(double jerk) {
    physics->set_max_velocity((last_feed > jerk) ? (last_feed - jerk) : jerk);
}
