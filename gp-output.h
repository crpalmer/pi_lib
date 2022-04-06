#ifndef __GP_OUTPUT_H__
#define __GP_OUTPUT_H__

#include <assert.h>

#include "io.h"
#include "externals/PIGPIO/pigpio.h"

class GPOutput : public output_t {
public:

    GPOutput(unsigned gpio) {
	this->gpio = gpio;
        gpioSetMode(gpio, PI_OUTPUT);
    }

    void set(bool value) {
        gpioWrite(gpio, value);
    }

private:
    unsigned gpio;
};

#endif
