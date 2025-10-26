#include "pi.h"
#include "tb6612.h"

TB6612::TB6612(Output *standby, Output *dir1, Output *dir2, Output *pwm) : standby(standby), dir1(dir1), dir2(dir2), pwm(pwm) {
    standby->off();
    direction(true);
    pwm->pwm_enable(50*1000);
}

void TB6612::direction(bool forward) {
    if (forward) {
	dir1->on();
	dir2->off();
    } else {
	dir1->off();
	dir2->on();
    }
}

void TB6612::speed(double speed) {
    if (speed <= 0) {
	standby->off();
	pwm->pwm(0);
    } else {
	standby->on();
	pwm->pwm(speed);
    }
}
