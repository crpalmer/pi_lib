#include <stdio.h>
#include <stdlib.h>

#include <hardware/gpio.h>
#include "pico-servo.h"
#include "pi-gpio.h"

#define NUM_GPIOS 40

typedef struct {
   pi_gpio_irq_handler_t f;
   void *arg;
   alarm_id_t alarm_id;
   unsigned last_event;
} irq_handler_state_t;

typedef struct {
    unsigned gpio;
    enum { G_FREE = 0, G_IN, G_OUT, G_PWM, G_SERVO } state;
    irq_handler_state_t irq_handler;
} gpio_t;

static gpio_t gpios[NUM_GPIOS];

void pi_gpio_init()
{
    servo_init();
    servo_clock_auto();
    for (int i = 0; i < NUM_GPIOS; i++) {
	gpios[i].gpio = i;
	gpios[i].irq_handler.alarm_id = -1;
    }
}

int pi_gpio_get(unsigned gpio)
{
    if (gpios[gpio].state == G_FREE) gpios[gpio].state = G_IN;
    assert(gpios[gpio].state == G_IN);

    return gpio_get(gpio);
}

int pi_gpio_set(unsigned gpio, uint8_t value)
{
    if (gpios[gpio].state == G_FREE) gpios[gpio].state = G_OUT;
    assert(gpios[gpio].state == G_OUT);

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
    gpios[gpio].state = (mode == PI_OUTPUT ? G_OUT : G_IN);
    return 0;
}

static bool irq_enabled = false;

unsigned translate_events(uint32_t events)
{
    unsigned new_events = 0;

    if (events & GPIO_IRQ_EDGE_RISE) new_events = PI_GPIO_EVENT_RISING;
    if (events & GPIO_IRQ_EDGE_FALL) new_events = PI_GPIO_EVENT_FALLING;

    return new_events;
}

static int64_t gpio_irq_alarm(alarm_id_t id, void *gpio_as_vp)
{
    gpio_t *gpio = (gpio_t *) gpio_as_vp;
    irq_handler_state_t *irq = &gpio->irq_handler;

    irq->alarm_id = -1;
    irq->f(irq->arg, gpio->gpio, translate_events(irq->last_event));
    return 0;
}

static void gpio_callback(uint gpio, uint32_t events) {
    irq_handler_state_t *irq = &gpios[gpio].irq_handler;

    if (irq->f) {
	if (irq->alarm_id >= 0) cancel_alarm(irq->alarm_id);
	irq->last_event = events;
	if ((irq->alarm_id = add_alarm_in_us(1000, gpio_irq_alarm, &gpios[gpio], true)) < 0) {
	    fprintf(stderr, "pi-gpio irq_handler: Failed to create alarm for debouncing\n");
            irq->f(irq->arg, gpio, translate_events(events));
	}
    }
}

int pi_gpio_set_irq_handler(unsigned gpio, pi_gpio_irq_handler_t irq_handler, void *irq_handler_arg)
{
    irq_handler_state_t *irq = &gpios[gpio].irq_handler;

    irq->f = irq_handler;
    irq->arg = irq_handler_arg;

    if (! irq_enabled) {
	gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
	irq_enabled = true;
    } else {
	gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    }

    return 0;
}

int pi_gpio_servo(unsigned gpio, unsigned ms)
{
    if (gpios[gpio].state == G_FREE) {
	servo_attach(gpio);
	gpios[gpio].state = G_SERVO;
    } else if (gpios[gpio].state != G_SERVO) {
	return -1; 
    }

    return servo_microseconds(gpio, ms);
}

