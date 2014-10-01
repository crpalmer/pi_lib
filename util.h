#ifndef __UTIL_H__
#define __UTIL_H__

#define bool int
#define true 1
#define false 0

void
seed_random();

double
random_number();

bool
randomly_with_prob(double prob);

void
ms_sleep(unsigned ms);

#endif

