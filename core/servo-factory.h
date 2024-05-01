#ifndef __SERVO_FACTORY_H__
#define __SERVO_FACTORY_H__

#include "servo.h"

class ServoFactory {
public:
     virtual int get_n_servos() = 0;
     virtual Servo *get_servo(unsigned id) = 0;
};

#endif
