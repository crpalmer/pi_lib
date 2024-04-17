#ifndef __SNTP_H__
#define __SNTP_H__

#include <time.h>

int
net_sntp_time(const char *host, struct timespec *now);

#endif
