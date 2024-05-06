#ifndef __SERVO_GPIO_H__
#define __SERVO_GPIO_H__

#include "mem.h"
#include "pi-gpio.h"
#include "servo.h"
#include "servo-factory.h"

class GpioServo : public Servo {
public:
    GpioServo(unsigned pin, unsigned mn = SERVO_STANDARD_MIN, unsigned mx = SERVO_STANDARD_MAX) : pin(pin), mn(mn), mx(mx) { }

    bool move_to(double pos) override {
	if (pos < 0) pos = 0;
	if (pos > 100) pos = 100;

	if (is_inverted) pos = 100 - pos;
	double us = (pos / 100) * (mx - mn) + mn;
	pi_gpio_servo(pin, us);
	return true;
    }

    bool set_range(unsigned mn, unsigned mx) override {
	if (mn >= mx) return false;
	this->mn = mn;
	this->mx = mx;
	return true;
    }

    bool set_is_inverted(bool is_inverted) override {
	this->is_inverted = is_inverted;
	return true;
    }

private:
    unsigned pin, mn, mx;
    bool is_inverted = false;
};

class GpioServoFactory : public ServoFactory {
public:
    GpioServoFactory(int *user_pins, size_t n_pins) : n_pins(n_pins) {
	pins = (int *) fatal_malloc(sizeof(*pins) * n_pins);
	for (size_t i =0 ; i < n_pins; i++) pins[i] = user_pins[i];
    }

    ~GpioServoFactory() {
	fatal_free(pins);
    }

    Servo *get_servo(unsigned id) override {
	if (id >= n_pins) return NULL;
	return new GpioServo(pins[id]);
    }

    int get_n_servos() override {
	return (int) n_pins;
    }

private:
    int *pins;
    size_t n_pins;
};

#endif
