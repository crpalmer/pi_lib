#ifndef __STEPPER_H__
#define __STEPPER_H__

#include "io.h"
#include "pi-threads.h"
#include "time-utils.h"

class Stepper : public PiThread {
public:
    Stepper(Output *enable, Output *dir, Output *step, const char *name = "stepper", int priority = 1) : PiThread(name), name(name), enable(enable), dir(dir), step(step) {
	lock = new PiMutex();
	cond = new PiCond();

	start(priority);
    }

    void main(void) override;

    int64_t get_n_steps();
    void reset_n_steps();

    void set_speed(double mm_per_sec);
    void set_acceleration(double mm_per_sec2);
    void set_jerk(double mm_per_sec);
    void set_steps_per_mm(double steps_per_mm);

    void dump_state();

private:
    const char *name;
    Output *enable, *dir, *step;
    PiMutex *lock;
    PiCond *cond;

    double v = 0;
    double target_v = 0;
    double acceleration = 100;
    double jerk = 2.5;
    double steps_per_mm = 200;

    int64_t n_steps = 0;

private:
    us_time_t one_step(us_time_t next_step);
};

#endif
