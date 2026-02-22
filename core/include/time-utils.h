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

typedef uint64_t nano_time_t;

/// \endcond

static inline int nano_elapsed_ms(const nano_time_t *newer, const nano_time_t *later);

/** Fills the timespec with the current time.
 *
 * The time is as accurate as the system clock allows, down to nanosecond
 * precision.
 *
 * \param t - Location to store the time data
 */

void nano_gettime(nano_time_t *t);

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

void nano_sleep_until(nano_time_t *t);

/** Sleep for a specified number of milliseconds.
 *
 * \param ms - The duration (in ms) to sleep
 */

void ms_sleep(unsigned ms);

/** Sleep for a specified number of nanoseconds.
 *
 * Note: not currently implemented for the raspberry pi.
 *
 * \param us - The duration (in us) to sleep
 */

void us_sleep(unsigned us);

/** Add the specified number of microseconds to a timespec.
 *
 * \param t - The timespec to increase
 * \param usec - The number of microseconds to add
 */

static inline void
nano_add_usec(nano_time_t *t, unsigned usec) {
    (*t) += usec;
}

/** Add the specified number of milliseconds to a timespec.
 *
 * \param t - The timespec to increase
 * \param ms - The number of milliseconds to add
 */

static inline void
nano_add_ms(nano_time_t *t, unsigned ms) {
    (*t) += ms * 1000;
}

/** Compares two timespecs for relative ordering.
 *
 * \param now
 * \param then
 *
 * \returns \c true iff \c now is later then \c then.
 */

static inline int
nano_later_than(const nano_time_t *now, const nano_time_t *then) {
    return (*now) > (*then);
}

/** Compares a timespec against the current time.
 *
 * \param then - The time to check
 *
 * \returns \c true iff \c then is in the past.
 */

static inline int
nano_now_is_later_than(const nano_time_t *then)
{
    nano_time_t now;

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
nano_elapsed_ms(const nano_time_t *newer, const nano_time_t *later) {
    return (*newer) - (*later);
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
nano_elapsed_ms_now(const nano_time_t *start) {
    nano_time_t now;

    nano_gettime(&now);
    return nano_elapsed_ms(&now, start);
}

#ifdef __cplusplus
};
#endif

#endif
