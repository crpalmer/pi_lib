#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pi-gpio.h"
#include "time-utils.h"
#include "util.h"

#include "wb.h"

#define N_INPUTS 8
#define N_OUTPUTS_PER_BANK 8

#define WB_OUTPUT(bank, pin) (((bank-1))*8 + (pin-1) + N_INPUTS)

typedef struct {
    const char *name;
    int id;
    unsigned mode;
} gpio_table_t;

#define N_GPIO_TABLE 24

static gpio_table_t gpio_table1[N_GPIO_TABLE] = {
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

static gpio_table_t gpio_table2[N_GPIO_TABLE] = {
    { "in1", 23, PI_INPUT },
    { "in2", 18, PI_INPUT },
    { "in3", 15, PI_INPUT },
    { "in4", 14, PI_INPUT },
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

static gpio_table_t *gpio_table = gpio_table1;

static bool wb_is_init = false;

static inline unsigned
get_input_id(unsigned pin)
{
    assert(1 <= pin && pin <= N_INPUTS);
    return pin - 1;
}

static inline unsigned
get_output_id(unsigned bank, unsigned pin)
{
    assert(1 <= bank && bank <= 2);
    assert(1 <= pin && pin <= N_OUTPUTS_PER_BANK);

    return (bank-1)*N_OUTPUTS_PER_BANK + (pin-1) + N_INPUTS;
}

static int
wb_init_common(void)
{
    int i;

    if (wb_is_init) return 0;

    pi_gpio_init();

    for (i = 0; i < N_GPIO_TABLE; i++) {
	pi_gpio_set_direction(gpio_table[i].id, gpio_table[i].mode);
	if (gpio_table[i].mode == PI_INPUT) {
	    pi_gpio_set_pullup(gpio_table[i].id, PI_PUD_DOWN);
	}
    }

    wb_is_init = true;

    return 0;
}

int
wb_init()
{
    gpio_table = gpio_table1;
    return wb_init_common();
}

int
wb_init_v2()
{
    gpio_table = gpio_table2;
    return wb_init_common();
}

bool wb_get(unsigned pin)
{
    int id = get_input_id(pin);
    return pi_gpio_get(gpio_table[id].id);
}

unsigned
wb_get_all(void)
{
    size_t pin;
    unsigned ret = 0;

    for (pin = 1; pin <= N_INPUTS; pin++) {
	ret |= (wb_get(pin) << (pin-1));
    }

    return ret;
}

unsigned
wb_get_all_with_debounce(unsigned debounce_ms)
{
    unsigned val;
    struct timespec start;

    nano_gettime(&start);
    val = wb_get_all();
    while (1) {
	unsigned cur = wb_get_all();
	if (cur != val) {
	    val = cur;
	    nano_gettime(&start);
	} else if (nano_elapsed_ms_now(&start) >= debounce_ms) {
	    return val;
	}
    }
}

void
wb_set(unsigned bank, unsigned pin, unsigned value)
{
    int id = get_output_id(bank, pin);

    if (gpio_table[id].mode != PI_OUTPUT) {
	pi_gpio_set_direction(gpio_table[id].id, PI_OUTPUT);
	gpio_table[id].mode = PI_OUTPUT;
    }
    pi_gpio_set(gpio_table[id].id, value);
}

void
wb_set_outputs(unsigned mask, unsigned values)
{
    int bank, pin;
    unsigned cur_bit = 1;

    for (bank = 1; bank <= 2; bank++) {
	for (pin = 1; pin <= 8; pin++, cur_bit <<= 1) {
	    if ((mask & cur_bit) != 0) {
		wb_set(bank, pin, (values & cur_bit) != 0);
	    }
	}
    }
}

void
wb_set_pull_up(unsigned pin, wb_pull_up_mode_t mode)
{
    int id = get_input_id(pin);

    switch(mode) {
    case WB_PULL_UP_NONE:
	pi_gpio_set_pullup(gpio_table[id].id, PI_PUD_OFF);
	break;
    case WB_PULL_UP_DOWN:
	pi_gpio_set_pullup(gpio_table[id].id, PI_PUD_DOWN);
	break;
    case WB_PULL_UP_UP:
	pi_gpio_set_pullup(gpio_table[id].id, PI_PUD_UP);
	break;
    }
}

#define DEFAULT_DEBOUNCE_MS 2


unsigned wb_wait_for_pins_full(unsigned pins, unsigned values, unsigned debounce_ms, unsigned max_ms)
{
    unsigned yes[N_INPUTS] = { 0, };
    struct timespec yes_start[N_INPUTS];
    struct timespec start, now;

    nano_gettime(&start);

    while (true) {
	unsigned pin;

        nano_gettime(&now);

	for (pin = 1; pin <= N_INPUTS; pin++) {
	    unsigned value = (values >> pin) & 1;
	    if (! (pins & (1<<pin))) continue;
	    if (wb_get(pin) == value) {
		if (! yes[pin]) {
		    yes[pin] = 1;
		    yes_start[pin] = now;
		}
		if (nano_elapsed_ms(&now, &yes_start[pin]) >= debounce_ms) {
		    return pin;
		}
	    } else {
		yes[pin] = 0;
	    }
	    if (max_ms != -1 && nano_elapsed_ms(&now, &start) > max_ms) return 0;
	}
    }
}

unsigned wb_wait_for_pins_timeout(unsigned pins, unsigned values, unsigned max_ms)
{
    return wb_wait_for_pins_full(pins, values, DEFAULT_DEBOUNCE_MS, max_ms);
}

unsigned wb_wait_for_pins(unsigned pins, unsigned values)
{
     return wb_wait_for_pins_timeout(pins, values, -1);
}

bool wb_wait_for_pin_timeout(unsigned pin, unsigned value, unsigned max_ms)
{
    return wb_wait_for_pins_timeout(WB_PIN_MASK(pin), value ? WB_PIN_MASK(pin) : 0, max_ms);
}

void wb_wait_for_pin(unsigned pin, unsigned value)
{
    wb_wait_for_pin_timeout(pin, value, -1);
}
