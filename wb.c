#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pigpio.h"
#include "util.h"

#include "wb.h"

struct wbS {
};

#define N_INPUTS 8
#define N_OUTPUTS 16

static struct {
    const char *name;
    int id;
    unsigned default_mode;
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

wb_t *
wb_new(void)
{
    wb_t *w;
    int i;

    if (gpioInitialise() < 0) return NULL;

    w = malloc(sizeof(*w));

    for (i = 0; i < ARRAY_SIZE(gpio_table); i++) {
	gpioSetMode(gpio_table[i].id, gpio_table[i].default_mode);
    }

    return w;
}

bool wb_get(wb_t *w, unsigned pin)
{
    assert(pin < N_INPUTS);
    return gpioRead(gpio_table[pin].id);
}

unsigned
wb_get_all(wb_t *w)
{
    size_t pin;
    unsigned ret = 0;

    for (pin = 0; pin < N_INPUTS; pin++) {
	ret |= (wb_get(w, pin) << pin);
    }

    return ret;
}

void
wb_set(wb_t *w, unsigned pin, unsigned value)
{
    assert(pin < N_OUTPUTS);
    gpioWrite(gpio_table[pin + N_INPUTS].id, value);
}

void
wb_destroy(wb_t *w)
{
    free(w);
}
