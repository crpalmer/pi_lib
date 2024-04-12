#ifndef __UTIL_H__
#define __UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

void
seed_random();

double
random_number();

unsigned
random_number_in_range(unsigned low, unsigned high);

double
random_double_in_range(double low, double high);

bool
randomly_with_prob(double prob);

#ifdef __cplusplus
};
#endif

#endif

