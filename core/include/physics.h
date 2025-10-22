#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include <cstdio>
#include <cmath>
#include "time-utils.h"

class PhysicsClock {
public:
    virtual ~PhysicsClock() {};
    virtual void start() = 0;
    virtual double get() = 0;
};

class PhysicsTimeClock : public PhysicsClock {
    ~PhysicsTimeClock() override {}
    void start() override { nano_gettime(&start_time); }
    double get() override { return nano_elapsed_ms_now(&start_time) / 1000.0; }

private:
    struct timespec start_time;
};

class Physics {
public:
    Physics(double a = 1, double max_v = 1, PhysicsClock *clock = NULL);

    void set_acceleration(double new_a);
    void set_max_velocity(double new_max_v);

    double start_motion(double pos0, double pos_n);

    double get_pos();
    double get_v();
    double get_deceleration_t();

    bool is_done();

private:
    void update_model();
    double get_pos_unlimited();
    double raw_pos_at_t(double t);
    double raw_t_at_pos(double pos);

private:
    PhysicsClock *clock;

    double a, max_v;
    double start_pos, target_pos;
    double t_of_max_v;
    double deccel_pos;
    double deccel_v;
    double deccel_t;
};

#endif
