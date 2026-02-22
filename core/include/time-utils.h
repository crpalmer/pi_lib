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

typedef uint64_t us_time_t;

/// \endcond

static inline void us_sleep(us_time_t us);
static inline int us_elapsed_ms(const us_time_t *newer, const us_time_t *later);

static inline void ms_sleep(unsigned int ms) {
    us_sleep(ms*(us_time_t)1000);
}

#include "platform-time-utils.h"

/** Add the specified number of milliseconds to a timespec.
 *
 * \param t - The timespec to increase
 * \param ms - The number of milliseconds to add
 */

static inline void us_add_ms(us_time_t *t, unsigned ms) {
    (*t) += ms * 1000;
}

/** Compares a timespec against the current time.
 *
 * \param then - The time to check
 *
 * \returns \c true iff \c then is in the past.
 */

static inline int us_now_is_later_than(const us_time_t *then)
{
    us_time_t now;

    us_gettime(&now);
    return now > *then;
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

static inline int us_elapsed_ms(const us_time_t *newer, const us_time_t *later) {
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

static inline int us_elapsed_ms_now(const us_time_t *start) {
    us_time_t now;

    us_gettime(&now);
    return us_elapsed_ms(&now, start);
}

#ifdef __cplusplus
};
#endif

#endif
