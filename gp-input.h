#ifndef __GP_INPUT_H__
#define __GP_INPUT_H__

#include <assert.h>

#include "io.h"
#include "pi-gpio.h"

static void on_change_wrapper(void *self, unsigned gpio_unused, unsigned events_unused);

class GPInput : public input_t {
public:

    GPInput(unsigned gpio) : input_t() {
	this->gpio = gpio;
        gpioSetMode(gpio, PI_INPUT);
    }

    unsigned get_fast() {
	return gpioRead(gpio) == 0;
    }

    void set_pullup_up() {
	gpioSetPullUpDown(gpio, PI_PUD_UP);
    }

    void set_pullup_down() {
	gpioSetPullUpDown(gpio, PI_PUD_DOWN);
    }

    void clear_pullup() {
        gpioSetPullUpDown(gpio, PI_PUD_OFF);
    }

    int enable_irq() {
	return pi_gpio_set_irq_handler(gpio, on_change_wrapper, this);
    }

    virtual void on_change(bool is_rising, bool is_falling) { on_change(); }

    virtual void on_change() { }

private:
    unsigned gpio;
};

static void on_change_wrapper(void *self, unsigned gpio_unused, unsigned events)
{
    GPInput *input = (GPInput *) self;
    input->on_change((events & PI_GPIO_EVENT_RISING) != 0, (events & PI_GPIO_EVENT_FALLING) != 0);
}

#endif
