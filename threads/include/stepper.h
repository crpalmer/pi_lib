#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <cfloat>
#include "io.h"
#include "pi-threads.h"

class Stepper : PiThread {
public:
    Stepper(Output *dir, Output *step, double steps_per_mm, const char *name = "stepper");
    ~Stepper();

    void main() override;

    void set_endstop(Input *end_stop, double low, double high, bool is_low = true);
    void set_acceleration(double mm_per_sec_squared);
    void set_jerk(double mm_per_sec);

    void go(double pos_mm, double feed = 0, bool async = true);

private:
    Output *dir, *step;
    Input *end_stop = NULL;

    double steps_per_mm;
    double mm_per_step;
    double low = 0;
    double high = DBL_MAX;
    bool is_low = true;
    double accel = 1000;
    double jerk = 100;
    double last_feed = 100;

    double pos = 0;
    double target = 0;
    bool   active = false;

    PiMutex *lock;
    PiCond *cond;
};

#endif
