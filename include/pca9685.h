#ifndef __PCA9685_H__
#define __PCA9685_H__

#include "io.h"

class PCA9685_Output;

class PCA9685 {
    friend PCA9685_Output;

public:
    PCA9685(unsigned address = 0x40, unsigned hz = 1024);
    ~PCA9685();

    Output *get_output(unsigned id);

protected:
    void set(unsigned id, double pct);

private:
    void reset();
    void set_pwm_freq(unsigned hz);

    unsigned hz;
    int i2c;
};

class PCA9685_Output : public Output {
public:
    PCA9685_Output(PCA9685 *parent, unsigned id) : parent(parent), id(id) {}
    virtual void set(bool value) override { parent->set(id, value ? 1.0 : 0.0); }
    virtual void pwm(double pct) override { parent->set(id, pct); }

private:
    PCA9685 *parent;
    unsigned id;
};

#endif
