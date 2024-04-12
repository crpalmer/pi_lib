#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <pthread.h>
#include <linux/gpio.h>
#include "pi-gpio.h"
#include "time-utils.h"

#define NUM_GPIOS 40

typedef struct {
    unsigned gpio;
    struct gpiod_line *line;
    enum { G_FREE = 0, G_IN, G_OUT, G_SERVO} state;
    unsigned bias;
    struct {
        pthread_t t;
        pi_gpio_irq_handler_t f;
        void *arg;
        bool active;
    } irq_handler;
} gpio_t;

static gpio_t gpios[NUM_GPIOS];

static const char *chip_name = NULL;
static struct gpiod_chip *chip = NULL;

static struct gpiod_chip *try_chip(const char *name)
{
   struct gpiod_chip *c = gpiod_chip_open_by_name(name);
   if (c) {
	struct gpiod_line *line = gpiod_chip_find_line(c, "GPIO27");
	if (line) {
	    gpiod_line_release(line);
	    fprintf(stderr, "pi-gpio: using chip %s (version %s)\n", name, gpiod_version_string());
	    chip_name = name;
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

    gpios[gpio].gpio = gpio;
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

    switch (updown) {
    case PI_PUD_OFF: gpios[gpio].bias = GPIOD_LINE_REQUEST_FLAG_BIAS_DISABLE; break;
    case PI_PUD_UP: gpios[gpio].bias = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP; break;
    case PI_PUD_DOWN: gpios[gpio].bias = GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN; break;
    default:
	fprintf(stderr, "Invalid updown %d\n", updown);
	return -1;
    }
     
    if (gpiod_line_set_flags(line, gpios[gpio].bias) < 0) {
	perror("gpiod_line_set_flags");
	return -1;
    }

    return 0;
}

int pi_gpio_set_direction(unsigned gpio, unsigned mode)
{
    assert_state(gpio, mode == PI_INPUT ? G_IN : G_OUT);
    if (mode == PI_INPUT) pi_gpio_set_pullup(gpio, PI_PUD_OFF);
    return 0;
}

static void *irq_handler_main(void *gpio_as_vp)
{
    gpio_t *gpio = (gpio_t *) gpio_as_vp;

    while (true) {
	struct gpiod_line_event event;

	if (gpiod_line_event_read(gpio->line, &event) < 0) {
	    perror("gpiod_line_event_read");
	    ms_sleep(1000);
	} else {
	    struct timespec ts;

	    ts.tv_sec = 0;
	    ts.tv_nsec = 1*1000;

	    while (gpiod_line_event_wait(gpio->line, &ts) > 0) {
		if (gpiod_line_event_read(gpio->line, &event) < 0) {
		    perror("gpiod_line_event_read");
		}
	    }
	    gpio->irq_handler.f(gpio->irq_handler.arg, gpio->gpio, event.event_type == GPIOD_LINE_EVENT_RISING_EDGE ? PI_GPIO_EVENT_RISING : PI_GPIO_EVENT_FALLING);
	}
    }

    return NULL;
}

int pi_gpio_set_irq_handler(unsigned gpio, pi_gpio_irq_handler_t irq_handler, void *irq_handler_arg)
{
    if (gpios[gpio].irq_handler.active) {
	/* Already running, just change the handler and arg */

	/* There is a slight race condition here where an event could occur and
         * call the new handler with the old arg, but that's really unlikely and
         * I'm not sure you should really be redefining the handler anyway.
         */
	gpios[gpio].irq_handler.f = irq_handler;
	gpios[gpio].irq_handler.arg = irq_handler_arg;
	return 0;
    }

    struct gpiod_line *line = get_line(gpio);
    if (! line) return -1;

    gpiod_line_release(line);
    if (gpiod_line_request_both_edges_events_flags(line, "pi-gpio-pwm", gpios[gpio].bias) < 0) return -1;

    gpios[gpio].irq_handler.f = irq_handler;
    gpios[gpio].irq_handler.arg = irq_handler_arg;

    if (pthread_create(&gpios[gpio].irq_handler.t, NULL, irq_handler_main, &gpios[gpio]) < 0) return -1;
    gpios[gpio].irq_handler.active = true;

    return 0;
}
