#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "gpio.h"
#include "util.h"

#include "wb.h"

struct wbS {
    gpio_t *gpio;
};

#define N_INPUTS 8
#define N_OUTPUTS 16

static gpio_table_t gpio_table[] = {
    { "in1", 0, GPIO_IS_INPUT },
    { "in2", 0, GPIO_IS_INPUT },
    { "in3", 0, GPIO_IS_INPUT },
    { "in4", 0, GPIO_IS_INPUT },
    { "in5", 0, GPIO_IS_INPUT },
    { "in6", 0, GPIO_IS_INPUT },
    { "in7", 0, GPIO_IS_INPUT },
    { "in8", 0, GPIO_IS_INPUT },
    { "out1_1", 0, 0 },
    { "out1_2", 0, 0 },
    { "out1_3", 0, 0 },
    { "out1_4", 0, 0 },
    { "out1_5", 0, 0 },
    { "out1_6", 0, 0 },
    { "out1_7", 0, 0 },
    { "out1_8", 0, 0 },
    { "out2_1", 0, 0 },
    { "out2_2", 0, 0 },
    { "out2_3", 0, 0 },
    { "out2_4", 0, 0 },
    { "out2_5", 0, 0 },
    { "out2_6", 0, 0 },
    { "out2_7", 0, 0 },
    { "out2_8", 0, 0 },
};

wb_t *
wb_new(void)
{
    wb_t *w = malloc(sizeof(*w));

    w->gpio = gpio_new(gpio_table, ARRAY_SIZE(gpio_table));

    return w;
}

bool wb_get(wb_t *w, unsigned pin)
{
    assert(pin < N_INPUTS);
    return gpio_get_id(w->gpio, pin);
}

unsigned
wb_get_all(wb_t *w)
{
    size_t pin;
    unsigned ret =0;

    for (pin = 0; pin < N_INPUTS; pin++) {
	ret |= (wb_get(w, pin) << pin);
    }

    return ret;
}

void
wb_set(wb_t *w, unsigned pin, unsigned value)
{
    assert(pin < N_OUTPUTS);
    gpio_set_id(w->gpio, pin + N_INPUTS, value);
}

void
wb_destroy(wb_t *w)
{
    gpio_destroy(w->gpio);
    free(w);
}
