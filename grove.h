#ifndef __GROVE_H__
#define __GROVE_H__

#include <assert.h>
#include "pigpio.h"
#include "motor.h"

class grove_motor_t;

class GroveDC {
public:
    GroveDC(unsigned addr);
    void direction(unsigned id, int dir);
    void speed(unsigned id, unsigned speed);
    motor_t *get_motor(unsigned id);

protected:
    void set_speed();
    void set_direction();
    void assert_id(unsigned id) { assert(id == 0 || id == 1); }

private:
    int bus;
    unsigned char dirs[2];
    unsigned char speeds[2];
};

class GroveStepper {
public:
    GroveStepper(unsigned addr, unsigned phases = 2);
    void direction(int dir);
    void step();

protected:
    void set_step();
    void set_direction(unsigned short dir);

private:
    int bus;
    int cur_step;
    int dir;
    unsigned phases;
};

class grove_motor_t : public motor_t {
public:
    grove_motor_t(GroveDC *g, unsigned id) : g(g), id(id) { }
    void speed(double speed) { g->speed(id, 255*speed); }
    void direction(bool forward) { g->direction(id, forward ? +1 : -1); }

private:
    GroveDC *g;
    unsigned id;
};

#endif
