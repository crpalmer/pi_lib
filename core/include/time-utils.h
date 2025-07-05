/** \file
 *
 * Time utilities
 *
 *  A collection of utilities to help work with system time.
 */

#ifndef __TIME_UTILS_COMMON_H__
#define __TIME_UTILS_COMMON_H__

/// \cond Internal

#include <time.h>
#include <assert.h>

/// \endcond

#ifdef __cplusplus
extern "C" {
#endif

/// \cond Internal

#define TIMESPEC_FMT "%d.%09d"
#define NANOSEC_PER_MS   1000000
#define USEC_PER_SEC	 1000000
#define NANOSEC_PER_USEC 1000
#define MS_PER_SEC       1000

/// \endcond

static inline int nano_elapsed_ms(const struct timespec *newer, const struct timespec *later);

/** Fills the timespec with the current time.
 *
 * The time is as accurate as the system clock allows, down to nanosecond
 * precision.
 *
 * \param t - Location to store the time data
 */

void nano_gettime(struct timespec *t);

/** Sleep until the specified timespec.
 *
 * Sleeps until the some time at or beyond the specified timespec.
 * There is no guarantee how close it will be to the specified time 
 * when this function returns.
 *
 * If the specified timespec is in the past, the function will immediately
 * return.
 *
 * \param t - The timespec to sleep until
 */

void nano_sleep_until(struct timespec *t);

/** Sleep for a specified number of milliseconds.
 *
 * \param ms - The duration (in ms) to sleep
 */

void ms_sleep(unsigned ms);

/** Add the specified number of microseconds to a timespec.
 *
 * \param t - The timespec to increase
 * \param usec - The number of microseconds to add
 */

static inline void
nano_add_usec(struct timespec *t, unsigned usec)
{
    t->tv_sec += usec / USEC_PER_SEC;
    t->tv_nsec += (usec % USEC_PER_SEC) * NANOSEC_PER_USEC;
    t->tv_sec += t->tv_nsec / (MS_PER_SEC*NANOSEC_PER_MS);
    t->tv_nsec = t->tv_nsec % (MS_PER_SEC*NANOSEC_PER_MS);
}

/** Add the specified number of milliseconds to a timespec.
 *
 * \param t - The timespec to increase
 * \param ms - The number of milliseconds to add
 */

static inline void
nano_add_ms(struct timespec *t, unsigned ms)
{
    t->tv_sec += ms / 1000;
    t->tv_nsec += (ms % 1000) * NANOSEC_PER_MS;
    t->tv_sec += t->tv_nsec / (MS_PER_SEC*NANOSEC_PER_MS);
    t->tv_nsec = t->tv_nsec % (MS_PER_SEC*NANOSEC_PER_MS);
}

/** Compares two timespecs for relative ordering.
 *
 * \param now
 * \param then
 *
 * \returns \c true iff \c now is later then \c then.
 */

static inline int
nano_later_than(const struct timespec *now, const struct timespec *then)
{
    return now->tv_sec > then->tv_sec || (now->tv_sec == then->tv_sec && now->tv_nsec >= then->tv_nsec);
}

/** Compares a timespec against the current time.
 *
 * \param then - The time to check
 *
 * \returns \c true iff \c then is in the past.
 */

static inline int
nano_now_is_later_than(const struct timespec *then)
{
    struct timespec now;

    nano_gettime(&now);
    return nano_later_than(&now, then);
}

/** Subtract one timespec from another.
 *
 * Returns the number of ms of the time specified as \c newer - \c later.
 *
 * It is undefined to if \c newer is actually earlier than \c later.
 *
 * \param newer - The later of the two timespecs
 * \param later - The earlier of the two timespecs
 */

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

/** Subtract a timespec from the current time.
 *
 * Returns the number of since \c start.
 *
 * It is undefined to if \c newer is actually earlier than \c later.
 *
 * \param start - A timespec of a past time
 */

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
