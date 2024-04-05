#ifndef __GP_OUTPUT_H__
#define __GP_OUTPUT_H__

#include <assert.h>

#include "io.h"
#include "pi-gpio.h"

class GPOutput : public output_t {
public:

    GPOutput(unsigned gpio) {
	this->gpio = gpio;
        pi_gpio_set_direction(gpio, PI_OUTPUT);
    }

    void set(bool value) {
        pi_gpio_set(gpio, value);
    }

private:
    unsigned gpio;
};

#endif
