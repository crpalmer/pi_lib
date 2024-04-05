#include <stdio.h>
#include <stdlib.h>

#include <hardware/gpio.h>
#include "pico_servo.h"
#include "pi-gpio.h"

#define NUM_GPIOS 40

static enum { G_FREE = 0, G_IN, G_OUT, G_PWM } gpios[NUM_GPIOS];

void pi_gpio_init()
{
    servo_init();
    servo_clock_auto();
}

int pi_gpio_get(unsigned gpio)
{
    if (gpios[gpio] == G_FREE) gpios[gpio] = G_IN;
    assert(gpios[gpio] == G_IN);

    return gpio_get(gpio);
}

int pi_gpio_set(unsigned gpio, uint8_t value)
{
    if (gpios[gpio] == G_FREE) gpios[gpio] = G_OUT;
    assert(gpios[gpio] == G_OUT);

    gpio_put(gpio, value);
    return 0;
}

int pi_gpio_set_pullup(unsigned gpio, unsigned updown)
{
    gpio_set_pulls(gpio, updown == PI_PUD_UP, updown == PI_PUD_DOWN);
    return 0;
}

int pi_gpio_set_direction(unsigned gpio, unsigned mode)
{
    gpio_init(gpio);
    pi_gpio_set_pullup(gpio, PI_PUD_OFF);
    gpio_set_dir(gpio, mode == PI_OUTPUT);
    gpios[gpio] = (mode == PI_OUTPUT ? G_OUT : G_IN);
    return 0;
}
