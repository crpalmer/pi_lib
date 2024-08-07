#ifndef __TIME_UTILS_COMMON_H__
#define __TIME_UTILS_COMMON_H__

#include <time.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TIMESPEC_FMT "%d.%09d"
#define NANOSEC_PER_MS   1000000
#define USEC_PER_SEC	 1000000
#define NANOSEC_PER_USEC 1000
#define MS_PER_SEC       1000

static inline int nano_elapsed_ms(const struct timespec *newer, const struct timespec *later);

void nano_gettime(struct timespec *t);
void nano_sleep_until(struct timespec *t);
void ms_sleep(unsigned ms);

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
nano_later_than(const struct timespec *now, const struct timespec *then)
{
    return now->tv_sec > then->tv_sec || (now->tv_sec == then->tv_sec && now->tv_nsec >= then->tv_nsec);
}

static inline int
nano_now_is_later_than(const struct timespec *then)
{
    struct timespec now;

    nano_gettime(&now);
    return nano_later_than(&now, then);
}

static inline int
nano_elapsed_ms(const struct timespec *newer, const struct timespec *later)
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
    printf("%ld.%010ld - %ld.%010ld = %d * 1000\n", (long) newer->tv_sec, (long) newer->tv_nsec, (long) later->tv_sec, (long) later->tv_nsec, ms);
#endif

    return ms;
}

static inline int
nano_elapsed_ms_now(const struct timespec *start)
{
    struct timespec now;

    nano_gettime(&now);
    return nano_elapsed_ms(&now, start);
}

#ifdef __cplusplus
};
#endif

#endif
