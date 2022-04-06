#ifndef __GP_INPUT_H__
#define __GP_INPUT_H__

#include <assert.h>

#include "io.h"
#include "externals/PIGPIO/pigpio.h"

class GPInput : public input_t {
public:

    GPInput(unsigned gpio) {
	this->gpio = gpio;
        gpioSetMode(gpio, PI_INPUT);
    }

    unsigned get() {
	return gpioRead(gpio) == 0;
    }

    void set_pullup_up() {
	gpioSetPullUpDown(gpio, PI_PUD_UP);
    }

    void set_pullup_down() {
	gpioSetPullUpDown(gpio, PI_PUD_DOWN);
    }

    void clear_pullup() {
        gpioSetPullUpDown(gpio, PI_PUD_OFF);
    }

private:
    unsigned gpio;
};

#endif
