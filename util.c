#include <stdio.h>
#include <stdlib.h>
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
    return random();
}

bool
randomly_with_prob(double prob)
{
    return random_number() < prob;
}

