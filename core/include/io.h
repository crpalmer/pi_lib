#ifndef __IO_H__
#define __IO_H__

#include "time-utils.h"

class Output {
public:
     virtual ~Output() {}
     virtual void set(bool value) = 0;
     virtual void pwm(double pct_on) { set(pct_on >= 0.5); }
     virtual void on() { set(is_inverted ? 0 : 1); }
     virtual void off() { set(is_inverted ? 1 : 0); }
     virtual void set_is_inverted(bool new_is_inverted = true) {
	is_inverted = new_is_inverted;
     }

private:
     bool is_inverted = false;
};

class InputNotifier {
public:
    virtual void on_change(bool is_rising, bool is_falling) { on_change(); }
    virtual void on_change() { }
};

class Input {
public:
     virtual ~Input() {}
     virtual unsigned get_fast() = 0;
     virtual void set_pullup_up() = 0;
     virtual void set_pullup_down() = 0;
     virtual void clear_pullup() = 0;

     void set_is_inverted(bool set_inverted = true) { is_inverted = set_inverted; }
     void set_debounce(unsigned ms) { debounce_ms = ms; }
 
     unsigned get() {
	if (debounce_ms) {
	    return get_with_debounce(debounce_ms);
	} else {
	    return return_value(get_fast());
	}
     }

     unsigned get_with_debounce(unsigned ms = 1) {
        struct timespec start;
	unsigned v = get_fast();

        nano_gettime(&start);
        while (1) {
	    double vv = get_fast();
	    if (v != vv) {
		v = vv;
		nano_gettime(&start);
            } else if (nano_elapsed_ms_now(&start) >= (int) ms) {
		return return_value(v);
	    }
        }
    }

    virtual bool set_notifier(InputNotifier *notifier) { return false; }

protected:
    unsigned return_value(unsigned v) {
	return is_inverted ? ! v : v;
    }

    unsigned debounce_ms = 0;
    bool is_inverted = false;
};

#endif
