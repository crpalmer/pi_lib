#ifndef __SERVO_H__
#define __SERVO_H__
#endif

#ifdef PI_PICO
#include "pico_servo.h"
#endif

#define SERVO_STANDARD_MIN 1050
#define SERVO_STANDARD_MAX 1950

#define SERVO_HS425BB_MIN   530
#define SERVO_HS425BB_MAX  2520

#define SERVO_STRETCH_MIN   900
#define SERVO_STRETCH_MAX  2100

#define SERVO_EXTENDED_MIN  500
#define SERVO_EXTENDED_MAX 2500

#define SERVO_ST

class Servo {
public:
    Servo(unsigned pin, unsigned mn = SERVO_STANDARD_MIN, unsigned mx = SERVO_STANDARD_MAX)
    {
	this->pin = pin;
	this->mn  = mn;
	this->mx  = mx;
    }

    void go(double pos)
    {
	if (pos < 0) pos = 0;
	if (pos > 1) pos = 1;

	pi_gpio_servo(pin, pos * (mx - mn) + mn);
    }

private:
    unsigned pin, mn, mx;

    double convert_to_extended_range(double pos) {
	double desired_us = mn + (mx - mn) * pos;
	double us_past_extended_min = desired_us - SERVO_EXTENDED_MIN;
	double extended_pos = us_past_extended_min / (SERVO_EXTENDED_MAX - SERVO_EXTENDED_MIN);
	return extended_pos;
    }
};
