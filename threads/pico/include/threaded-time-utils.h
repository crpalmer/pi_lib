#ifndef __THREADED_TIME_UTILS_H__
#define __THREADED_TIME_UTILS_H__

// TODO: provide a native freeftos implementation of these somehow

static inline void us_sleep(us_time_t us) {
    unsigned ms = us / 1000;
    if (ms >= 1) {
	void task_delay(unsigned int ms);
	task_delay(ms);
	us -= ms * 1000;
    }
    busy_wait_us(us);
}

static inline void us_sleep_until(us_time_t *t) {
    us_time_t now;
    us_gettime(&now);
    if (*t > now) us_sleep(*t - now);
}

#endif
