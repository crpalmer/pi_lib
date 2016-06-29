#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pigpio.h"
#include "util.h"

#include "wb.h"

#define N_INPUTS 8
#define N_OUTPUTS 16

#define WB_PI_SERVO (1<<29)
#define WB_PI_PWM (1<<30)

static struct {
    const char *name;
    int id;
    unsigned mode;
} gpio_table[] = {
    { "in1", 14, PI_INPUT },
    { "in2", 15, PI_INPUT },
    { "in3", 18, PI_INPUT },
    { "in4", 23, PI_INPUT },
    { "in5", 22, PI_INPUT },
    { "in6", 27, PI_INPUT },
    { "in7", 17, PI_INPUT },
    { "in8", 4, PI_INPUT },
    { "out1_1", 24, PI_OUTPUT },
    { "out1_2", 25, PI_OUTPUT },
    { "out1_3", 8, PI_OUTPUT },
    { "out1_4", 7, PI_OUTPUT },
    { "out1_5", 12, PI_OUTPUT },
    { "out1_6", 16, PI_OUTPUT },
    { "out1_7", 20, PI_OUTPUT },
    { "out1_8", 21, PI_OUTPUT },
    { "out2_1", 26, PI_OUTPUT },
    { "out2_2", 19, PI_OUTPUT },
    { "out2_3", 13, PI_OUTPUT },
    { "out2_4", 6, PI_OUTPUT },
    { "out2_5", 5, PI_OUTPUT },
    { "out2_6", 11, PI_OUTPUT },
    { "out2_7", 9, PI_OUTPUT },
    { "out2_8", 10, PI_OUTPUT },
};

int
wb_init(void)
{
    int i;

    if (gpioInitialise() < 0) return -1;

    for (i = 0; i < ARRAY_SIZE(gpio_table); i++) {
	gpioSetMode(gpio_table[i].id, gpio_table[i].mode);
    }

    return 0;
}

bool wb_get(unsigned pin)
{
    assert(pin >= 1 && pin <= N_INPUTS);
    return gpioRead(gpio_table[pin-1].id);
}

unsigned
wb_get_all(void)
{
    size_t pin;
    unsigned ret = 0;
    unsigned shift;

    for (pin = 1, shift = 0; pin <= N_INPUTS; pin++, shift++) {
	ret |= (wb_get(pin) << shift);
    }

    return ret;
}

void
wb_set(unsigned pin, unsigned value)
{
    int id = pin + N_INPUTS;

    assert(pin < N_OUTPUTS);
    if (gpio_table[id].mode != PI_OUTPUT) {
	gpioSetMode(gpio_table[id].id, PI_OUTPUT);
	gpio_table[id].mode = PI_OUTPUT;
    }
    gpioWrite(gpio_table[id].id, value);
}

#define DEFAULT_PWM_FREQ 1000
#define PWM_RANGE 1000

void
wb_pwm(unsigned pin, float duty)
{
     wb_pwm_freq(pin, DEFAULT_PWM_FREQ, duty);
}

void
wb_pwm_freq(unsigned pin, unsigned freq, float duty)
{
    int id = pin + N_INPUTS;

    assert(pin < N_OUTPUTS);
    assert(duty >= 0 && duty <= 1);
    if (gpio_table[id].mode != WB_PI_PWM) {
	gpio_table[id].mode = WB_PI_PWM;
	gpioSetPWMrange(gpio_table[id].id, PWM_RANGE);
    }
    gpioSetPWMfrequency(gpio_table[id].id, freq);
    gpioPWM(gpio_table[id].id, duty * PWM_RANGE);
}

void
wb_servo(unsigned pin, unsigned pulse_width)
{
    int id = pin + N_INPUTS;

    assert(pin < N_OUTPUTS);
    if (gpio_table[id].mode != WB_PI_SERVO) {
	gpio_table[id].mode = WB_PI_SERVO;
    }
    gpioServo(gpio_table[id].id, pulse_width);
}
