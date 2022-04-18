#ifndef __SERVO_H__
#define __SERVO_H__
#endif

#ifdef PI_PICO
#include "pico_servo.h"
#endif

#define SERVO_STANDARD_MIN 1000
#define SERVO_STANDARD_MAX 2000

class Servo {
public:
    Servo(unsigned pin, unsigned mn = SERVO_STANDARD_MIN, unsigned mx = SERVO_STANDARD_MAX)
    {
	this->pin = pin;
	this->mn  = mn;
	this->mx  = mx;
#ifdef PI_PICO
	servo_attach(pin);
#endif
    }

    void go(double pos)
    {
	if (pos < 0) pos = 0;
	if (pos > 1) pos = 1;

#ifdef PI_PICO
	servo_move_to(pin, pos * 180);
#else
	gpioServo(pin, pos * (mx - mn) + mn);
#endif
    }

private:
    unsigned pin, mn, mx;
};
