#ifndef __IO_H__
#define __IO_H__

class output_t {
public:
     ~output_t() {}
     virtual void set(unsigned value) = 0;
     virtual void pwm(double pct_on) { set(pct_on >= 0.5); }
};

class input_t {
public:
     ~input_t() {}
     virtual unsigned get() = 0;
     virtual void set_pullup_up() = 0;
     virtual void set_pullup_down() = 0;
     virtual void clear_pullup() = 0;
};

#endif
