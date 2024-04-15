#ifndef __ROTARY_ENcODER_H__
#define __ROTARY_ENCODER_H__

#include <limits.h>
#include "gp-input.h"

class RE_A_Input;

class RotaryEncoderNotifier {
public:
    virtual void on_change(int new_value) { };
    virtual void on_switch(bool pressed) { };
};

class RotaryEncoder : public GPInputNotifier {
public:
    RotaryEncoder(unsigned gpio_A, unsigned gpio_B, int gpio_switch = -1) {
	A = new GPInput(gpio_A);
	B = new GPInput(gpio_B);

	/* Add pull down resistors so it doesn't go crazy if the RE
 	 * is unplugged or otherwise loses power.
	 */
	A->set_pullup_down();
	B->set_pullup_down();

	if (gpio_switch >= 0) {
	    sw = new GPInput(gpio_switch);
	    sw->set_pullup_down();
	    last_sw = sw->get();
	    sw->set_notifier(this);
	}

	last_A = A->get();
	A->set_notifier(this);
    }

    void set_range(int min_value, int max_value) {
	this->min_value = min_value;
	this->max_value = max_value;

	if (value < min_value) value = min_value;
	if (value > max_value) value = max_value;
    }

    void set_notifier(RotaryEncoderNotifier *notifier) {
	this->notifier = notifier;
    }

    int get() { return value; }

    bool get_switch() { return sw ? sw->get() : false; }

    void set(int new_value) {
	if (min_value <= new_value && new_value <= max_value) value =  new_value;
    }

    virtual void on_change() {
	unsigned new_A = A->get();

	if (new_A != last_A) {
	    if (B->get() == new_A) {
		if (value < max_value) {
		    value++;
		    if (this->notifier) this->notifier->on_change(value);
		}
	    } else {
		if (value > min_value) {
		    value--;
		    if (this->notifier) this->notifier->on_change(value);
		}
	    }
	    last_A = new_A;
	}

	if (sw) {
	    unsigned new_sw = sw->get();
	    if (new_sw != last_sw) {
		if (this->notifier) this->notifier->on_switch(new_sw);
		last_sw = new_sw;
	    }
	}
    }

private:
    GPInput *A, *B, *sw;
    int min_value = -INT_MAX, max_value = INT_MAX;
    int value = 0;
    unsigned last_A;
    bool last_sw;
    RotaryEncoderNotifier *notifier;
};

#endif
