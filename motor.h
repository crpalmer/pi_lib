#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "util.h"

class motor_t {
public:
    ~motor_t() { }
    virtual void speed(double pct) = 0;
    virtual void direction(bool forward) = 0;

    void change_motor(bool direction, double speed)
    {
        this->speed(0);
        ms_sleep(50);
        this->direction(direction);
        this->speed(speed);
    }

    void stop()
    {
	this->speed(0);
	ms_sleep(50);
    }
};

#endif
