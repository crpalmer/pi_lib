#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <pthread.h>
#include <linux/gpio.h>
#include <pigpio.h>
#include "pi-gpio.h"
#include "util.h"

int pi_gpio_servo(unsigned gpio, unsigned ms)
{
    return gpioServo(gpio, ms);
}

