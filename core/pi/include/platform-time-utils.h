#ifndef __PLATFORM_TIME_UTILS_H__
#define __PLATFORM_TIME_UTILS_H__

static inline us_time_t us_now() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1000000LL) + (ts.tv_nsec / 1000);
}

static inline void us_gettime(us_time_t *t) {
    *t = us_now();
}

static inline void us_sleep_until(us_time_t t) {
    struct timespec ts;
    ts.tv_sec = t / 1000000;
    ts.tv_nsec = (t % 1000000) * 1000;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
}

static inline void us_sleep(us_time_t us) {
    us_sleep_until(us_now() + t);
}

#endif
