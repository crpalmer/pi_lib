#ifndef __MOTOR_H__
#define __MOTOR_H__

class motor_t {
public:
    ~motor_t() { }
    virtual void speed(double pct) = 0;
    virtual void direction(bool forward) = 0;
};

#endif
