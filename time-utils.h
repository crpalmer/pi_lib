#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#include <time.h>
#include <assert.h>

#define TIMESPEC_FMT "%d.%09d"
#define NANOSEC_PER_MS   1000000
#define USEC_PER_SEC	 1000000
#define NANOSEC_PER_USEC 1000
#define MS_PER_SEC       1000

static inline void
nano_gettime(struct timespec *t)
{
     clock_gettime(CLOCK_MONOTONIC, t);
}

static inline void
nano_add_usec(struct timespec *t, unsigned usec)
{
    t->tv_sec += usec / USEC_PER_SEC;
    t->tv_nsec += (usec % USEC_PER_SEC) * NANOSEC_PER_USEC;
    t->tv_sec += t->tv_nsec / (MS_PER_SEC*NANOSEC_PER_MS);
    t->tv_nsec = t->tv_nsec % (MS_PER_SEC*NANOSEC_PER_MS);
}

static inline void
nano_add_ms(struct timespec *t, unsigned ms)
{
    t->tv_sec += ms / 1000;
    t->tv_nsec += (ms % 1000) * NANOSEC_PER_MS;
    t->tv_sec += t->tv_nsec / (MS_PER_SEC*NANOSEC_PER_MS);
    t->tv_nsec = t->tv_nsec % (MS_PER_SEC*NANOSEC_PER_MS);
}

static inline int
nano_later_than(struct timespec *now, struct timespec *then)
{
    return now->tv_sec > then->tv_sec || (now->tv_sec == then->tv_sec && now->tv_nsec >= then->tv_nsec);
}

static inline int
nano_elapsed_ms(struct timespec *newer, struct timespec *later)
{
    int ms = (newer->tv_sec - later->tv_sec) * MS_PER_SEC;
    int nsec;

    if (newer->tv_nsec < later->tv_nsec) {
	nsec = newer->tv_nsec + (MS_PER_SEC*NANOSEC_PER_MS);
	ms -= MS_PER_SEC;
    } else {
	nsec = newer->tv_nsec;
    }

    ms += (nsec - later->tv_nsec) / NANOSEC_PER_MS;

#if 0
    printf("%ld.%010ld - %ld.%010ld = %d * 1000\n", newer->tv_sec, newer->tv_nsec, later->tv_sec, later->tv_nsec, ms);
#endif

    assert(ms >= 0);

    return ms;
}

static inline int
nano_elapsed_ms_now(struct timespec *start)
{
    struct timespec now;

    nano_gettime(&now);
    return nano_elapsed_ms(&now, start);
}

static inline void
nano_sleep_until(struct timespec *t)
{
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
}

#endif
