#include "pi.h"
#include "l298n.h"

L298N::L298N(Output *en, Output *dir1, Output *dir2) : en(en), dir1(dir1), dir2(dir2)
{
    en->set(0);
    dir1->set(0);
    dir2->set(1);
}

void L298N::speed(double speed)
{
    en->set(0);
    en->pwm(speed);
}

void L298N::direction(bool forward)
{
    if (forward) {
	dir1->set(0);
	dir2->set(1);
    } else {
	dir1->set(1);
	dir2->set(0);
    }
}
