#ifndef __LIGHTS_H__
#define __LIGHTS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct lightsS lights_t;

lights_t *lights_new(unsigned min_pin, unsigned max_pin);
void lights_chase(lights_t *);
void lights_on(lights_t *);
void lights_off(lights_t *);
void lights_select(lights_t *, unsigned selected);
void lights_blink(lights_t *);
void lights_blink_one(lights_t *l, unsigned pin);

#ifdef __cplusplus
};

#include "io.h"

#include <list>

class Action;

class Lights {
friend class BlinkAllAction;
friend class ChaseAction;

public:
    Lights();

    void add(output_t *light);
    void blink_one(output_t *light);
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

protected:
    void set_all(unsigned value);
    int blink_ms;

private:
    static void *work(void *this_as_vp);
    void set_action(Action *action);

    pthread_t        thread;
    pthread_mutex_t  lock;
    pthread_cond_t   cond;
    std::list<output_t *>  lights;
    unsigned	     blink_pin;
    Action	    *action;
};

#endif
#endif
