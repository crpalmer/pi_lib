#ifndef __GP_INPUT_H__
#define __GP_INPUT_H__

#include <assert.h>

#include "io.h"
#include "pi-gpio.h"

class GPInput : public Input {
public:
    GPInput(unsigned gpio) {
	this->gpio = gpio;
        pi_gpio_set_direction(gpio, PI_INPUT);
	pi_gpio_set_pullup(gpio, PI_PUD_OFF);
    }
    ~GPInput() { }

    unsigned get_fast() override {
	return pi_gpio_get(gpio) == 0;
    }

    void set_pullup_up() override {
	pi_gpio_set_pullup(gpio, PI_PUD_UP);
    }

    void set_pullup_down() override {
	pi_gpio_set_pullup(gpio, PI_PUD_DOWN);
    }

    void clear_pullup() override {
        pi_gpio_set_pullup(gpio, PI_PUD_OFF);
    }

    bool set_notifier(InputNotifier *notifier) override {
	this->notifier = notifier;
	return pi_gpio_set_irq_handler(gpio, GPInput::on_change_wrapper, this) == 0;
    }

    int get_gpio() { return gpio; }

private:
    static void on_change_wrapper(void *self, unsigned gpio_unused, unsigned events) {
	GPInput *input = (GPInput *) self;
	input->forward_on_change((events & PI_GPIO_EVENT_RISING) != 0, (events & PI_GPIO_EVENT_FALLING) != 0);
    }

    void forward_on_change(bool is_rising, bool is_falling) {
	notifier->on_change(is_rising, is_falling);
    }

private:
    unsigned gpio;
    InputNotifier *notifier;
};

#endif
