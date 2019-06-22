#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "util.h"
#include "digital-counter.h"
#include "wb.h"

#define RESET_PAUSE 100
#define POST_RESET_PAUSE 1000
#define PAUSE 10

digital_counter_t::digital_counter_t(output_t *inc, output_t *dec, output_t *reset) : inc(inc), dec(dec), reset(reset)
{
    pause = PAUSE;
    reset_pause = RESET_PAUSE;
    post_reset_pause = POST_RESET_PAUSE;

    stop = 0;
    target = actual = 0;

    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
printf("creating thread\n");
    pthread_create(&thread, NULL, thread_main, this);
}

digital_counter_t::~digital_counter_t(void)
{
    void *unused;
    stop = true;
    pthread_cond_signal(&cond);
    pthread_join(thread, &unused);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
}

void digital_counter_t::set(unsigned value)
{
    pthread_mutex_lock(&lock);
    target = value;
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&cond);
}

void digital_counter_t::add(int delta)
{
    pthread_mutex_lock(&lock);
    if (delta < 0 && target < -delta) target = 0;
    else target += delta;
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&cond);
}

void digital_counter_t::set_pause(int pause, int reset_pause, int post_reset_pause)
{
    if (pause > 0) this->pause = pause;
    if (reset_pause > 0) this->reset_pause = reset_pause;
    if (post_reset_pause > 0) this->post_reset_pause = post_reset_pause;
}

void *digital_counter_t::thread_main(void *this_as_vp) {
    digital_counter_t *dc = (digital_counter_t *) this_as_vp;

printf("created thread\n");
    pthread_mutex_lock(&dc->lock);
    while (! dc->stop) {
	int delta = dc->target - dc->actual;
	int abs_delta = fabs(delta);
	int step = delta > 0 ? 1 : -1;
	output_t *output = delta > 0 ? dc->inc : dc->dec;
	int i;

printf("target %u actual %u\n", dc->target, dc->actual);

	if (dc->target * 2*dc->pause + 2*dc->reset_pause + dc->post_reset_pause < abs_delta * 2*dc->pause || (delta < 0 && ! dc->dec)) {
	     /* Faster to just reset and go or else impossible */
	    dc->actual = 0;
	    ms_sleep(dc->reset_pause);
	    dc->reset->set(1);
	    ms_sleep(dc->reset_pause);
	    dc->reset->set(0);
	    ms_sleep(dc->post_reset_pause);
	    continue;
	} 

	pthread_mutex_unlock(&dc->lock);

	if (abs_delta > 5) abs_delta = 5;
	for (i = 0; i < abs_delta; i++) {
	    output->set(1);
	    ms_sleep(dc->pause);
	    output->set(0);
	    ms_sleep(dc->pause);
	}

	pthread_mutex_lock(&dc->lock);
	dc->actual += step * i;
	while (dc->target == dc->actual && ! dc->stop) {
	    pthread_cond_wait(&dc->cond, &dc->lock);
	}
    }

    return NULL;
}

#define C extern "C"

C digital_counter_t *
digital_counter_new(unsigned bank, unsigned inc, unsigned dec, unsigned reset)
{
    return new digital_counter_t(wb_get_output(bank, inc), wb_get_output(bank, dec), wb_get_output(bank, reset));
}

C void
digital_counter_set_pause(digital_counter_t *dc, int pause, int reset_pause, int post_reset_pause)
{
    dc->set_pause(pause, reset_pause, post_reset_pause);
}

C void
digital_counter_add(digital_counter_t *dc, int n)
{
    dc->add(n);
}

C void
digital_counter_set(digital_counter_t *dc, int new_value)
{
    dc->set(new_value);
}

C void digital_counter_reset(digital_counter_t *dc)
{
    digital_counter_set(dc, 0);
}

C void digital_counter_free(digital_counter_t *dc)
{
    delete dc;
}
