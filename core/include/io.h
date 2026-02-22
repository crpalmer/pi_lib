#ifndef __IO_H__
#define __IO_H__

#include "time-utils.h"

class Output {
public:
     virtual ~Output() {}
     void set(bool value) { set_internal(is_inverted ? !value : value); }
     virtual void pwm_enable(unsigned hz) { }
     virtual void pwm(double pct_on) { set(pct_on >= 0.5); }
     virtual void pwm_disable() { }
     virtual void on() { set(true); }
     virtual void off() { set(false); }
     virtual void set_is_inverted(bool new_is_inverted = true) {
	is_inverted = new_is_inverted;
     }

protected:
    virtual void set_internal(bool value) = 0;

private:
     bool is_inverted = false;
};

class DummyOutput : public Output {
public:
    void set_internal(bool value) override {}
    void pwm(double pct) override {}
    void on() override {}
    void off() override {}
    void set_is_inverted(bool b) {}
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
        us_time_t start;
	unsigned v = get_fast();

        us_gettime(&start);
        while (1) {
	    double vv = get_fast();
	    if (v != vv) {
		v = vv;
		us_gettime(&start);
            } else if (us_elapsed_ms_now(&start) >= (int) ms) {
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
