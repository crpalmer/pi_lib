#ifndef __L298N_H__
#define __L298N_H__

#include "io.h"
#include "motor.h"

class L298N : public motor_t {
public:
    L298N(Output *en, Output *dir1, Output *dir2);
    void speed(double) override;
    void direction(bool forward) override;

private:
    Output *en, *dir1, *dir2;
};

#endif
