#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include "random-utils.h"

void
seed_random()
{
    srand(time(NULL));
}

double
random_number()
{
    return rand() / (RAND_MAX + 1.0);
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
    return rand() < (prob * LONG_MAX);
}

