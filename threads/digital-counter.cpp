#include "pi.h"
#include <math.h>
#include "digital-counter.h"
#include "wb.h"

#define RESET_PAUSE 100
#define POST_RESET_PAUSE 1000
#define PAUSE 10

DigitalCounter::DigitalCounter(Output *inc, Output *dec, Output *reset, const char *name) : PiThread(name), inc(inc), dec(dec), reset(reset) {
    pause = PAUSE;
    reset_pause = RESET_PAUSE;
    post_reset_pause = POST_RESET_PAUSE;

    stop = 0;
    target = 0;
    actual = 0xffff;

    inc->set(0);
    if (dec) dec->set(0);
    reset->set(0);

    lock = new PiMutex();
    cond = new PiCond();

    start();
}

DigitalCounter::~DigitalCounter(void) {
    stop = true;
    delete cond;
    delete lock;
}

void DigitalCounter::set(unsigned value) {
    lock->lock();
    target = value;
    lock->unlock();
    cond->signal();
}

void DigitalCounter::add(int delta) {
    lock->lock();
    if (delta < 0 && target < -delta) target = 0;
    else target += delta;
    lock->unlock();
    cond->signal();
}

void DigitalCounter::set_pause(int pause, int reset_pause, int post_reset_pause) {
    if (pause > 0) this->pause = pause;
    if (reset_pause > 0) this->reset_pause = reset_pause;
    if (post_reset_pause > 0) this->post_reset_pause = post_reset_pause;
}

void DigitalCounter::main() {
    lock->lock();
    while (! stop) {
	int delta = target - actual;
	int abs_delta = fabs(delta);
	int step = delta > 0 ? 1 : -1;
	Output *output = delta > 0 ? inc : dec;
	int i;

	if (target * 2*pause + 2*reset_pause + post_reset_pause < abs_delta * 2*pause || (delta < 0 && ! dec)) {
	     /* Faster to just reset and go or else impossible */
	    actual = 0;
	    ms_sleep(reset_pause);
	    reset->set(1);
	    ms_sleep(reset_pause);
	    reset->set(0);
	    ms_sleep(post_reset_pause);
	    continue;
	} 

	lock->unlock();

	if (abs_delta > 5) abs_delta = 5;
	for (i = 0; i < abs_delta; i++) {
	    output->set(1);
	    ms_sleep(pause);
	    output->set(0);
	    ms_sleep(pause);
	}

	lock->lock();

	actual += step * i;
	while (target == actual && ! stop) {
	    cond->wait(lock);
	}
    }
}
