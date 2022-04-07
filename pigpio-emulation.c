#include <stdio.h>
#include <stdlib.h>

#include "hardware/gpio.h"
#include "externals/PIGPIO/pigpio.h"

int gpioRead(unsigned gpio)
{
    return gpio_get(gpio);
}

int gpioWrite(unsigned gpio, unsigned value)
{
    gpio_put(gpio, value);
    return 0;
}

int gpioSetPullUpDown(unsigned gpio, unsigned updown)
{
    gpio_set_pulls(gpio, updown == PI_PUD_UP, updown == PI_PUD_DOWN);
    return 0;
}

int gpioSetMode(unsigned gpio, unsigned mode)
{
    gpio_init(gpio);
    gpio_set_pulls(gpio, 0, 0);
    gpio_set_dir(gpio, mode == PI_OUTPUT);
    return 0;
}

int gpioInitialize()
{
    return 0;
}

int gpioInitialise()
{
    return 0;
}

