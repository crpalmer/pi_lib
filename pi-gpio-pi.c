#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <linux/gpio.h>
#include "pi-gpio.h"

#define NUM_GPIOS 40

static struct {
    struct gpiod_line *line;
    enum { G_FREE = 0, G_IN, G_OUT} state;
} gpios[NUM_GPIOS];

static struct gpiod_chip *chip = NULL;

static struct gpiod_chip *try_chip(const char *name)
{
   struct gpiod_chip *c = gpiod_chip_open_by_name(name);
   if (c) {
	struct gpiod_line *line = gpiod_chip_find_line(c, "GPIO27");
	if (line) {
	    gpiod_line_release(line);
	    fprintf(stderr, "pi-gpio: using chip %s (version %s)\n", name, gpiod_version_string());
	    return c;
	}
	gpiod_chip_close(c);
    }
    return NULL;
}

void pi_gpio_init()
{
    assert(! chip);
    if (! chip) chip = try_chip("gpiochip0");
    if (! chip) chip = try_chip("gpiochip4");
    if (! chip) {
	fprintf(stderr, "Failed to find a gpiochip!\n");
	exit(0);
    }
}

static struct gpiod_line *get_line(unsigned gpio)
{
    char name[32];

    assert(chip);
    assert(gpio < NUM_GPIOS);

    if (gpios[gpio].line) return gpios[gpio].line;

    sprintf(name, "GPIO%d", gpio);
    gpios[gpio].line = gpiod_chip_find_line(chip, name);
    return gpios[gpio].line;
}

static void assert_state(unsigned gpio, unsigned state)
{
    assert(chip);
    assert(gpio < NUM_GPIOS);
    if (gpios[gpio].state == G_FREE) {
	struct gpiod_line *line = get_line(gpio);
	assert(line);
	gpios[gpio].state = state;
	int ret = 0;
	if (state == G_IN) ret = gpiod_line_request_input(line, "pi-gpio");
	if (state == G_OUT) ret = gpiod_line_request_output(line, "pi-gpio", 0);
	assert(ret == 0);
    }
    assert(gpios[gpio].state == state);
}

int pi_gpio_get(unsigned gpio)
{
    assert_state(gpio, G_IN);
    struct gpiod_line *line = get_line(gpio);
    if (! line) return -1;
    return gpiod_line_get_value(line);
}

int pi_gpio_set(unsigned gpio, uint8_t value)
{
    assert_state(gpio, G_OUT);
    struct gpiod_line *line = get_line(gpio);
    if (! line) return -1;
    int ret = gpiod_line_set_value(line, value);
    return ret;
}

int pi_gpio_set_pullup(unsigned gpio, unsigned updown)
{
    assert_state(gpio, G_IN);

    struct gpiod_line *line = get_line(gpio);
    if (! line) return -1;

    int bias;
    switch (updown) {
    case PI_PUD_OFF: bias = GPIOD_LINE_REQUEST_FLAG_BIAS_DISABLE; break;
    case PI_PUD_UP: bias = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP; break;
    case PI_PUD_DOWN: bias = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN; break;
    default:
	fprintf(stderr, "Invalid updown %d\n", updown);
	return -1;
    }
     
    if (gpiod_line_set_flags(line, bias) < 0) {
	perror("gpiod_line_set_flags");
	return -1;
    }
printf("now %d\n", gpiod_line_bias(line));

    return 0;
}

int pi_gpio_set_direction(unsigned gpio, unsigned mode)
{
    assert_state(gpio, mode == PI_INPUT ? G_IN : G_OUT);
    if (mode == PI_INPUT) pi_gpio_set_pullup(gpio, PI_PUD_OFF);
    return 0;
}
