#ifndef __GROVE_H__
#define __GROVE_H__

#include <assert.h>
#include "pigpio.h"

class GroveDC {
public:
    GroveDC(unsigned addr);
    void direction(unsigned id, int dir);
    void speed(unsigned id1, unsigned speed);

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

#endif
