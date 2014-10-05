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

bool
randomly_with_prob(double prob)
{
    return random() < (prob * LONG_MAX);
}

