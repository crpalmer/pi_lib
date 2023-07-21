#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "util.h"

#ifdef PI_PICO
#include "pico/time.h"
#endif

void
ms_sleep_os(unsigned ms)
{
#if PI_PICO
    sleep_ms(ms);
#else
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&ts, NULL);
#endif
}

void
seed_random()
{
    srandom(time(NULL));
}

double
random_number()
{
    return random() / (LONG_MAX + 1.0);
}

unsigned
random_number_in_range(unsigned low, unsigned high)
{
    return random_number() * (high - low + 1) + low;
}

double
random_double_in_range(double low, double high)
{
    return random_number() * (high - low) + low;
}

bool
randomly_with_prob(double prob)
{
    return random() < (prob * LONG_MAX);
}

