#include "time-utils.h"

void nano_gettime(nano_time_t *t) {
    clock_gettime(CLOCK_MONOTONIC, t);
}

void nano_sleep_until(nano_time_t *t) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

void ms_sleep(unsigned ms) {
    nano_time_t ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&ts, NULL);
}

void us_sleep(unsigned us) {
    nano_time_t t;
    nano_gettime(&t);
    nano_add_usec(&t, us);
    nano_sleep_until(&t);
}
