#include <stdio.h>
#include <list>
#include <assert.h>
#include "mem.h"
#include "pi-threads.h"
#include "random-utils.h"
#include "wb.h"

#include "lights.h"

class Action {
public:
    virtual ~Action() {}
    virtual void step() = 0;
};

class BlinkOneAction : public Action {
public:
    BlinkOneAction(Output *light) : light(light), state(1)
    {}

    void step() {
	light->set(state);
	state = !state;
    }

private:
    Output *light;
    int      state;
};

class BlinkAllAction : public Action {
public:
    BlinkAllAction(Lights *lights) : lights(lights), state(1)
    {}

    void step() {
	lights->set_all(state);
	state = !state;
    }

private:
    Lights *lights;
    Output *light;
    int      state;
};

class BlinkRandomAction : public Action {
public:
    BlinkRandomAction(Lights *lights) : lights(lights), picked(NULL)
    {
	n = 0;

	l = new Output*[lights->lights.size()];
	for (Output *&o : lights->lights) {
	    l[n++] = o;
	}
    }

    void step() {
	if (picked) picked->set(0);
	picked = l[random_number_in_range(0, n-1)];
	picked->set(1);
    }

private:
    Lights *lights;
    Output **l;
    Output *picked;
    unsigned n;
};

class ChaseAction : public Action {
public:
    ChaseAction(std::list<Output *> lights) : lights(lights), last(NULL)
    {
	it = this->lights.begin();
    }

    void step() {
	if (last) {
	    last->off();
	    it++;
	    if (it == lights.end()) it = lights.begin();
	}
	last = *it;
	last->on();
    }

private:
    std::list<Output *> lights;
    std::list<Output *>::iterator it;
    Output *last;
};

void
Lights::add(Output *light)
{
    assert(light);
    lights.push_back(light);
}

void
Lights::set_all(unsigned value)
{
    std::list<Output *> :: iterator it;

    for (it = lights.begin(); it != lights.end(); it++) {
	(*it)->set(value);
    }
}

void
Lights::work(void *this_as_vp)
{
    Lights *l = (Lights *) this_as_vp;

    pi_mutex_lock(l->lock);
    while (true) {
	if (l->action == NULL) {
	    pi_cond_wait(l->cond, l->lock);
	} else {
	    l->action->step();
	    pi_mutex_unlock(l->lock);
	    ms_sleep(l->blink_ms);
	    pi_mutex_lock(l->lock);
	}
    }
}

Lights::Lights()
{
    action = NULL;

    blink_ms = 200;
    lock = pi_mutex_new();
    cond = pi_cond_new();

    pi_thread_create("light-work", work, this);
}
    
void
Lights::set_action(Action *new_action)
{
    pi_mutex_lock(lock);
    if (action) delete action;
    action = new_action;
    pi_cond_signal(cond);
    pi_mutex_unlock(lock);
}

void
Lights::blink_all()
{
    set_action(new BlinkAllAction(this));
}

void
Lights::blink_one(Output *l)
{
    set_action(new BlinkOneAction(l));
}

void
Lights::blink_random()
{
    set_action(new BlinkRandomAction(this));
}

void
Lights::chase()
{
    if (lights.size() > 1) set_action(new ChaseAction(lights));
    else blink_all();
}
