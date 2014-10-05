#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "util.h"

void
ms_sleep(unsigned ms)
{
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&ts, NULL);
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

bool
randomly_with_prob(double prob)
{
    return random() < (prob * LONG_MAX);
}

