#ifndef __GP_OUTPUT_H__
#define __GP_OUTPUT_H__

#include <assert.h>

#include "io.h"
#include "pi-gpio.h"

class GPOutput : public Output {
public:

    GPOutput(unsigned gpio) {
	this->gpio = gpio;
        pi_gpio_set_direction(gpio, PI_OUTPUT);
    }

    void set_internal(bool value) override {
        pi_gpio_set(gpio, value);
    }

    void pwm_enable(unsigned freq_hz) override { pi_gpio_pwm_enable(gpio, freq_hz); }
    void pwm_disable() override { pi_gpio_pwm_disable(gpio); }
    void pwm(double pct) override { pi_gpio_pwm_set_duty(gpio, pct); }

private:
    unsigned gpio;
};

#endif
