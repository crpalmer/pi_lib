#ifndef __THREADED_TIME_UTILS_H__
#define __THREADED_TIME_UTILS_H__

static inline void us_sleep_until(us_time_t t) {
    extern bool pico_threads_initialized;

    us_time_t now = us_now();
    if (now >= t) return;

    us_time_t us = t - now;

    if (! pico_threads_initialized) {
	sleep_us(t - now);
	return;
    }

    unsigned ms = us / 1000;

    /* Scheduler sleep to the ms increment */
    if (ms >= 1) {
	void task_delay(unsigned int ms);
	task_delay(ms);
    }

    /* Busy wait the remaining time, if any */
    while (t < us_now()) {
    }
}

static inline void us_sleep(us_time_t us) {
    us_sleep_until(us_now() + us);
}

#endif
