#ifndef __PLATFORM_TIME_UTILS_H__
#define __PLATFORM_TIME_UTILS_H__

#include "pi.h"
#include "pico/time.h"
#include "pico/types.h"

static inline us_time_t us_now() {
    return get_absolute_time();
}

static inline void us_gettime(us_time_t *t) {
    *t = us_now();
}

#ifdef USING_LIBPI_THREADS
#include "threaded-time-utils.h"
#else

static inline void us_sleep_until(us_time_t *t) {
    us_time_t now;
    int ms;

    us_gettime(&now);
    ms = us_elapsed_ms(t, &now);
    if (ms > 0) sleep_ms(ms);
}

static inline void us_sleep(us_time_t us) {
    unsigned ms = us / 1000;
    if (ms >= 1) {
	sleep_ms(ms);
	us -= ms * 1000;
    }
    busy_wait_us(us);
}

#endif

#endif
