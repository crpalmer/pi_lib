#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include "util.h"
#include "wb.h"
#include "digital-counter.h"

struct digital_counterS {
    int pause, reset_pause, post_reset_pause;
    int bank;
    int inc, dec, reset;
    int target, actual;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    pthread_t thread;
    int stop;
};

#define RESET_PAUSE 100
#define POST_RESET_PAUSE 1000
#define PAUSE 10

static void *
thread_main(void *dc_as_vp)
{
    digital_counter_t *dc = (digital_counter_t *) dc_as_vp;

    pthread_mutex_lock(&dc->lock);
    while (! dc->stop) {
	int delta = dc->target - dc->actual;
	int abs_delta = fabs(delta);
	int step = delta > 0 ? 1 : -1;
	int pin = delta > 0 ? dc->inc : dc->dec;
	int i;

	if (dc->target * 2*dc->pause + 2*dc->reset_pause + dc->post_reset_pause < abs_delta * 2*dc->pause || (delta < 0 && dc->dec < 0)) {
	     /* Faster to just reset and go or else impossible */
	    dc->actual = 0;
	    ms_sleep(dc->reset_pause);
	    wb_set(dc->bank, dc->reset, 1);
	    ms_sleep(dc->reset_pause);
	    wb_set(dc->bank, dc->reset, 0);
	    ms_sleep(dc->post_reset_pause);
	    continue;
	} 

	pthread_mutex_unlock(&dc->lock);

	if (abs_delta > 5) abs_delta = 5;
	for (i = 0; i < abs_delta; i++) {
	    wb_set(dc->bank, pin, 1);
	    ms_sleep(dc->pause);
	    wb_set(dc->bank, pin, 0);
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

digital_counter_t *
digital_counter_new(int bank, int inc, int dec, int reset)
{
    digital_counter_t *dc = malloc(sizeof(digital_counter_t));

    dc->pause = PAUSE;
    dc->reset_pause = RESET_PAUSE;
    dc->post_reset_pause = POST_RESET_PAUSE;

    dc->target = 0;
    dc->actual = 99999;
    dc->bank = bank;
    dc->inc = inc;
    dc->dec = dec;
    dc->reset = reset;
    dc->stop = 0;

    pthread_mutex_init(&dc->lock, NULL);
    pthread_cond_init(&dc->cond, NULL);
    pthread_create(&dc->thread, NULL, thread_main, dc);

    digital_counter_reset(dc);

    return dc;
}

void
digital_counter_set_pause(digital_counter_t *dc, int pause, int reset_pause, int post_reset_pause)
{
    if (pause > 0) dc->pause = pause;
    if (reset_pause > 0) dc->reset_pause = reset_pause;
    if (post_reset_pause > 0) dc->post_reset_pause = post_reset_pause;
}

void
digital_counter_add(digital_counter_t *dc, int n)
{
    pthread_mutex_lock(&dc->lock);
    dc->target += n;
    pthread_mutex_unlock(&dc->lock);
    pthread_cond_signal(&dc->cond);
}

void
digital_counter_set(digital_counter_t *dc, int new_value)
{
    pthread_mutex_lock(&dc->lock);
    dc->target = new_value;
    pthread_mutex_unlock(&dc->lock);
    pthread_cond_signal(&dc->cond);
}

void digital_counter_reset(digital_counter_t *dc)
{
    digital_counter_set(dc, 0);
}

void digital_counter_free(digital_counter_t *dc)
{
    dc->stop = 1;
    pthread_cond_signal(&dc->cond);
    pthread_join(dc->thread, NULL);
    pthread_mutex_destroy(&dc->lock);
    pthread_cond_destroy(&dc->cond);
    free(dc);
}