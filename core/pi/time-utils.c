#include "pi.h"
#include "time-utils.h"

void nano_gettime(nano_time_t *t) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    (*t) = (ts.tv_sec * 1000LL) + (ts.tv_nsec / 1000);
}

void nano_sleep_until(nano_time_t *t) {
    struct timespec ts;
    ts.tv_sec = (*t) / 1000000;
    ts.tv_nsec = (*t % 1000000) * 1000;
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
}

void ms_sleep(unsigned ms) {
    struct timespec ts;

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
