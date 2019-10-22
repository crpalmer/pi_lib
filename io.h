#ifndef __IO_H__
#define __IO_H__

#include "time-utils.h"

class output_t {
public:
     ~output_t() {}
     virtual void set(bool value) = 0;
     virtual void pwm(double pct_on) { set(pct_on >= 0.5); }
     virtual void on() { set(1); }
     virtual void off() { set(0); }
};

class input_t {
public:
     ~input_t() {}
     virtual unsigned get() = 0;
     virtual void set_pullup_up() = 0;
     virtual void set_pullup_down() = 0;
     virtual void clear_pullup() = 0;
     virtual void set_debounce(unsigned ms) { }
 
    unsigned get_with_debounce(unsigned ms = 1) {
        struct timespec start;
	unsigned v = get();

        nano_gettime(&start);
        while (1) {
	    double vv = get();
	    if (v != vv) {
		v = vv;
		nano_gettime(&start);
            } else if (nano_elapsed_ms_now(&start) >= (int) ms) return v;
        }
    }
};

#endif
