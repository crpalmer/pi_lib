#ifndef __SNTP_H__
#define __SNTP_H__

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int
net_sntp_time(const char *host, struct timespec *now);

int
net_sntp_set_pico_rtc(const char *host);

#ifdef __cplusplus
};
#endif


#endif
