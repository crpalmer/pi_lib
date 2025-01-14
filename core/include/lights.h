#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#include "io.h"
#include "pi-threads.h"

#include <list>

class Light {
public:
    Light(Output *output = NULL) : output(output) {
    }

    void on() { if (output) output->on(); }
    void off() { if (output) output->off(); }
    void set(bool on) { if (output) output->set(on); }

private:
    Output *output;
};

class Action;

class Lights {
friend class BlinkAllAction;
friend class BlinkRandomAction;
friend class ChaseAction;

public:
    Lights();

    void add(Light *light);
    void blink_one(Light *light);
    void blink_all();
    void chase();
    void set_blink_ms(int blink_ms) { this->blink_ms = blink_ms; }

    void off()
    {
	set_all(0);
	set_action(NULL);
    }

    void on()
    {
	set_all(1);
	set_action(NULL);
    }

    void blink_random();

protected:
    void set_all(unsigned value);
    int blink_ms;

private:
    static void work(void *this_as_vp);
    void set_action(Action *action);

    pi_mutex_t      *lock;
    pi_cond_t       *cond;
    std::list<Light *>  lights;
    unsigned	     blink_pin;
    Action	    *action;
};

#endif
