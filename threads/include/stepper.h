#ifndef __STEPPER_H__
#define __STEPPER_H__

#include <cfloat>
#include "io.h"
#include "pi-threads.h"
#include "picostepper.h"

class Stepper : public PiThread {
public:
    Stepper(int base_pin, double steps_per_mm, const char *name = "stepper");
    ~Stepper();

    void main() override;

    void set_end_stop(Input *end_stop);
    bool home(double homed_pos = 0, double feed = 100, double mm_per_check = 0.1);

    void set_acceleration(double mm_per_sec_squared);
    void set_jerk(double mm_per_sec);

    void go(double pos_mm, double feed = 0, bool async = true);

    double get_pos() { return pos; }

private:
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

    PicoStepper device;
    PiMutex *lock;
    PiCond *cond;
};

#endif
