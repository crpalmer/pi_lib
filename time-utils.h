#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#include <time.h>

#define TIMESPEC_FMT "%d.%09d"
#define NANOSEC_PER_MS 1000000
#define MS_PER_SEC     1000

static inline void
nano_gettime(struct timespec *t)
{
     clock_gettime(CLOCK_MONOTONIC, t);
}

static inline void
nano_add_ms(struct timespec *t, unsigned ms)
{
    t->tv_nsec += ms * NANOSEC_PER_MS;
    t->tv_sec += t->tv_nsec / (MS_PER_SEC*NANOSEC_PER_MS);
    t->tv_nsec = t->tv_nsec % (MS_PER_SEC*NANOSEC_PER_MS);
}

static inline void
nano_sleep_until(struct timespec *t)
{
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

#endif
