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

static bool irq_enabled = false;
static struct {
    pi_gpio_irq_handler_t f;
    void *arg;
} irq_handlers[NUM_GPIOS];

unsigned translate_events(uint32_t events)
{
    unsigned new_events = 0;

    if (events & GPIO_IRQ_EDGE_RISE) new_events = PI_GPIO_EVENT_RISING;
    if (events & GPIO_IRQ_EDGE_FALL) new_events = PI_GPIO_EVENT_FALLING;

    return new_events;
}

static void gpio_callback(uint gpio, uint32_t events) {
    if (irq_handlers[gpio].f) irq_handlers[gpio].f(irq_handlers[gpio].arg, gpio, translate_events(events));
}

int pi_gpio_set_irq_handler(unsigned gpio, pi_gpio_irq_handler_t irq_handler, void *irq_handler_arg)
{
    irq_handlers[gpio].f = irq_handler;
    if (! irq_enabled) {
	gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	irq_enabled = true;
    } else {
	gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    }
    irq_handlers[gpio].f = irq_handler;
    irq_handlers[gpio].arg = irq_handler_arg;

    return 0;
}
