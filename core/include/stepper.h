#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <cfloat>
#include "io.h"
#include "physics.h"

class Stepper {
public:
    Stepper(Output *dir, Output *step, double steps_per_mm);
    ~Stepper();

    void home(Input *end_stop, bool forward, double homed_pos = 0, double feed = 100);
    void go(double pos_mm, double feed = 0);
    void set_acceleration(double mm_per_sec_squared);
    void set_jerk(double mm_per_sec);


    double get_pos() { return pos; }

private:
    double steps_per_mm;
    double mm_per_step;
    double accel = 1000;
    double jerk = 10;

    double last_feed = 100;
    double pos = 0;

    PhysicsClock *clock;
    Physics *physics;

    Output *dir;
    Output *step;
};

#endif
