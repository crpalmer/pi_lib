#include <stdio.h>
#include <pthread.h>
#include <malloc.h>
#include <list>
#include <assert.h>
#include "mem.h"
#include "util.h"
#include "wb.h"

#include "lights.h"

using namespace std;

class Action {
public:
    virtual ~Action() {}
    virtual void step() = 0;
};

class BlinkOneAction : public Action {
public:
    BlinkOneAction(output_t *light) : light(light), state(1)
    {}

    void step() {
	light->set(state);
	state = !state;
    }

private:
    output_t *light;
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
    output_t *light;
    int      state;
};

class ChaseAction : public Action {
public:
    ChaseAction(list<output_t *> lights) : lights(lights), last(NULL)
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
    list<output_t *> lights;
    list<output_t *>::iterator it;
    output_t *last;
};

void
Lights::add(output_t *light)
{
    assert(light);
    lights.push_back(light);
}

void
Lights::set_all(unsigned value)
{
    list<output_t *> :: iterator it;

    for (it = lights.begin(); it != lights.end(); it++) {
	(*it)->set(value);
    }
}

void *
Lights::work(void *this_as_vp)
{
    Lights *l = (Lights *) this_as_vp;

    pthread_mutex_lock(&l->lock);
    while (true) {
	if (l->action == NULL) {
	    pthread_cond_wait(&l->cond, &l->lock);
	} else {
	    l->action->step();
	    pthread_mutex_unlock(&l->lock);
	    ms_sleep(l->blink_ms);
	    pthread_mutex_lock(&l->lock);
	}
    }
    return NULL;
}

Lights::Lights()
{
    action = NULL;

    blink_ms = 200;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&thread, NULL, work, this);
}
    
void
Lights::set_action(Action *new_action)
{
    pthread_mutex_lock(&lock);
    if (action) delete action;
    action = new_action;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
}

void
Lights::blink_all()
{
    set_action(new BlinkAllAction(this));
}

void
Lights::blink_one(output_t *l)
{
    set_action(new BlinkOneAction(l));
}

void
Lights::chase()
{
    if (lights.size() > 1) set_action(new ChaseAction(lights));
    else blink_all();
}

/* C compatibility code */

struct lightsS {
    Lights *lights;
    output_t **outputs;
    unsigned min_pin;
};

lights_t *
lights_new(unsigned min_pin, unsigned max_pin)
{
    lights_t *lights = (lights_t *) fatal_malloc(sizeof(*lights));
    Lights *l = new Lights();

    lights->lights = l;
    lights->min_pin = min_pin;
    lights->outputs = (output_t **) fatal_malloc(sizeof(*lights->outputs) * (max_pin - min_pin + 1));

    for (unsigned pin = min_pin; pin <= max_pin; pin++) {
	lights->outputs[pin - min_pin] = wb_get_output(pin);
	l->add(lights->outputs[pin - min_pin]);
    }

    return lights;
}

void
lights_chase(lights_t *l)
{
    l->lights->chase();
}

void
lights_on(lights_t *l)
{
    l->lights->on();
}

void
lights_off(lights_t *l)
{
    l->lights->off();
}

void
lights_select(lights_t *l, unsigned selected)
{
    l->lights->off();
    wb_set(0, selected, 1);
}

void
lights_blink(lights_t *l)
{
    l->lights->blink_all();
}

void
lights_blink_one(lights_t *l, unsigned pin)
{
    l->lights->blink_one(l->outputs[pin - l->min_pin]);
}
