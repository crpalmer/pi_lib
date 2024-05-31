#ifndef __SERVOS_H__
#define __SERVOS_H__

#include <list>
#include "servo.h"

class Servos : public Servo {
public:
    void add(Servo *servo) {
	servos.push_back(servo);
    }

    bool move_to(double pos) override {
	bool res = true;
	for (Servo *servo : servos) res = servo->move_to(pos) && res;
	return res;
    }

    bool set_is_inverted(bool is_inverted) override {
	bool res = true;
	for (Servo *servo : servos) res = servo->set_is_inverted(is_inverted) && res;
	return res;
    }

    bool set_range(unsigned mn, unsigned mx) override {
	bool res = true;
	for (Servo *servo : servos) res = servo->set_range(mn, mx) && res;
	return res;
    }

    bool set_speed(unsigned ms_for_range) override {
	bool res = true;
	for (Servo *servo : servos) res = servo->set_speed(ms_for_range) && res;
	return res;
    }

private:
    std::list<Servo *> servos;
};

#endif
