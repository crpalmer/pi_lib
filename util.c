#define _POSIX_C_SOURCE 199506L
#include <stdio.h>
#include <time.h>

void
ms_sleep(unsigned ms)
{
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&ts, NULL);
}

