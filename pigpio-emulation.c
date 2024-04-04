#include <stdio.h>
#include <stdlib.h>

#include <hardware/gpio.h>
#include "pico_servo.h"
#include "pi-gpio.h"

#define NUM_GPIOS 40

static enum { G_FREE = 0, G_IN, G_OUT, G_PWM } gpios[NUM_GPIOS];

int gpioRead(unsigned gpio)
{
    if (gpios[gpio] == G_FREE) gpios[gpio] = G_IN;
    assert(gpios[gpio] == G_IN);

    return gpio_get(gpio);
}

int gpioWrite(unsigned gpio, unsigned value)
{
    if (gpios[gpio] == G_FREE) gpios[gpio] = G_OUT;
    assert(gpios[gpio] == G_OUT);

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
    gpios[gpio] = (mode == PI_OUTPUT ? G_OUT : G_IN);
    return 0;
}

int gpioPWM(unsigned user_gpio, unsigned dutycycle)
{
    assert(0);
}

int gpioInitialize()
{
    return gpioInitialise();
}

int gpioInitialise()
{
    servo_init();
    servo_clock_auto();

    return 0;
}
