#ifndef __GP_INPUT_H__
#define __GP_INPUT_H__

#include <assert.h>

#include "io.h"
#include "pi-gpio.h"

static void on_change_wrapper(void *self, unsigned gpio_unused, unsigned events_unused);

class GPInputNotifier {
public:
    virtual void on_change(bool is_rising, bool is_falling) { on_change(); }
    virtual void on_change() { }
};

class GPInput : public Input {
public:

    GPInput(unsigned gpio) : Input() {
	this->gpio = gpio;
        pi_gpio_set_direction(gpio, PI_INPUT);
    }

    unsigned get_fast() {
	return pi_gpio_get(gpio) == 0;
    }

    void set_pullup_up() {
	pi_gpio_set_pullup(gpio, PI_PUD_UP);
    }

    void set_pullup_down() {
	pi_gpio_set_pullup(gpio, PI_PUD_DOWN);
    }

    void clear_pullup() {
        pi_gpio_set_pullup(gpio, PI_PUD_OFF);
    }

    int set_notifier(GPInputNotifier *notifier) {
	this->notifier = notifier;
	return pi_gpio_set_irq_handler(gpio, on_change_wrapper, this);
    }

    void forward_on_change(bool is_rising, bool is_falling) {
	notifier->on_change(is_rising, is_falling);
    }

private:
    unsigned gpio;
    GPInputNotifier *notifier;
};

static void on_change_wrapper(void *self, unsigned gpio_unused, unsigned events)
{
    GPInput *input = (GPInput *) self;
    input->forward_on_change((events & PI_GPIO_EVENT_RISING) != 0, (events & PI_GPIO_EVENT_FALLING) != 0);
}

#endif
