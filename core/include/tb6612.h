#ifndef __TB6612_H__
#define __TB6612_H__

#include "io.h"
#include "motor.h"

class TB6612 : public Motor {
    TB6612(Output *standby, Output *dir1, Output *dir2, Output *pwm);
    void direction(bool forward) override;
    void speed(double speed) override;

private:
    Output *standby, *dir1, *dir2, *pwm;
};

#endif
