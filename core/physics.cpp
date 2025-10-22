#include "physics.h"

#include <cstdio>
#include <cmath>

Physics::Physics(double a, double max_v, PhysicsClock *clock) {
    if (clock) this->clock = clock;
    else this->clock = new PhysicsTimeClock();
    this->a = a;
    this->max_v = max_v;
    start_pos = target_pos = 0;
    update_model();
}

void Physics::set_acceleration(double new_a) {
    a = new_a;
    update_model();
}

void Physics::set_max_velocity(double new_max_v) {
    max_v = new_max_v;
    update_model();
}

double Physics::start_motion(double pos0, double pos_n) {
    clock->start();
    start_pos = pos0;
    target_pos = pos_n;
    update_model();
    if (deccel_t < t_of_max_v) return deccel_t * 2;
    return deccel_t + t_of_max_v;
}

double Physics::get_pos() {
    double pos = get_pos_unlimited();
    if (pos > target_pos) return target_pos;
    return pos;
}

double Physics::get_v() {
    double t = clock->get();

    if (t >= deccel_t) {
	double v = deccel_v - a * (t - deccel_t);
	return v > 0 ? v : 0;
    }

    double v = a*t;
    return v < max_v ? v : max_v;
}

bool Physics::is_done() {
    return get_pos() >= target_pos;
}

void Physics::update_model() {
    t_of_max_v = max_v / a;
    double pos_at_max_v = raw_pos_at_t(t_of_max_v);
    deccel_pos = target_pos - (pos_at_max_v - start_pos);
    if (deccel_pos < pos_at_max_v) {
	deccel_pos = (target_pos - start_pos) / 2 + start_pos;
	deccel_t = raw_t_at_pos(deccel_pos);
	deccel_v = deccel_t * a;
    } else {
	deccel_v = max_v;
	deccel_t = raw_t_at_pos(deccel_pos);
    }
    // printf("t_of_max_v = %f pos_at_max_v = %f deccel_pos %f deccel_v %f deccel_t %f\n", t_of_max_v, pos_at_max_v, deccel_pos, deccel_v, deccel_t);
}

double Physics::get_pos_unlimited() {
    double t = clock->get();

    if (t >= deccel_t) {
	double dt = t - deccel_t;

	if (deccel_v - a * dt < 0) return target_pos;
	return deccel_pos + deccel_v*dt - 0.5 * a * dt * dt;
    }

    if (t <= t_of_max_v) {
	return raw_pos_at_t(t);
    }

    return raw_pos_at_t(t_of_max_v) + (t - t_of_max_v) * max_v;
}

double Physics::raw_pos_at_t(double t) {
    return start_pos + 0.5 * a * t *t;
}

double Physics::raw_t_at_pos(double pos) {
    double pos_at_max_v = raw_pos_at_t(t_of_max_v);

    if (pos >= pos_at_max_v) {
	 return (pos - pos_at_max_v) / max_v + t_of_max_v;
    }
    return sqrt(2*(pos - start_pos) / a);
}
