#include "time-utils.h"

void nano_gettime(struct timespec *t) {
    clock_gettime(CLOCK_MONOTONIC, t);
}

void nano_sleep_until(struct timespec *t) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

void ms_sleep(unsigned ms) {
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&ts, NULL);
}
