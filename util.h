#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdbool.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void
seed_random();

double
random_number();

unsigned
random_number_in_range(unsigned low, unsigned high);

bool
randomly_with_prob(double prob);

void
ms_sleep(unsigned ms);

#endif

