#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <assert.h>

#define TIMESPEC_FMT "%d.%09d"
#define NANOSEC_PER_MS   1000000
#define USEC_PER_SEC	 1000000
#define NANOSEC_PER_USEC 1000
#define MS_PER_SEC       1000

#ifdef PI_PICO
#include <pico/time.h>
#include <pico/types.h>
#endif

static inline void
nano_gettime(struct timespec *t)
{
#ifdef PI_PICO
    absolute_time_t now = get_absolute_time();
    t->tv_sec = to_us_since_boot(now) / 1000 / 1000;
    t->tv_nsec = (to_us_since_boot(now) % (1000*1000)) * 1000;
#else
    clock_gettime(CLOCK_MONOTONIC, t);
#endif
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
nano_now_is_later_than(struct timespec *then)
{
    struct timespec now;

    nano_gettime(&now);
    return nano_later_than(&now, then);
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
#ifdef PI_PICO
    struct timespec now;
    int ms;

    nano_gettime(&now);
    ms = nano_elapsed_ms(t, &now);
    if (ms > 0) sleep_ms(ms);
#else
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, t, NULL);
#endif
}

#ifdef __cplusplus
};
#endif

#endif
