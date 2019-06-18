#ifndef __L298N_H__
#define __L298N_H__

#include "io.h"
#include "motor.h"

class L298N : public motor_t {
public:
    L298N(output_t *en, output_t *dir1, output_t *dir2);
    void speed(double) override;
    void direction(bool forward) override;

private:
    output_t *en, *dir1, *dir2;
};

#endif
