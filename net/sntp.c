#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "net.h"

#define	SNTP_SECS_AT_UNIX_EPOCH (2208988800)

static uint32_t
sntp_value(const uint8_t *ptr)
{
    return ((uint32_t) ptr[0]) << 24 |
           ((uint32_t) ptr[1]) << 16 |
           ((uint32_t) ptr[2]) <<  8 |
           ((uint32_t) ptr[3]);
}

#define NTP_SCALE_FRAC 4294967295.0

#include "time-utils.h"
int
net_sntp_time(const char *host, struct timespec *now)
{
    int ret = -1;
    int fd;

    if (host == NULL) host = "192.168.1.24";

    if ((fd = net_connect_udp(host, 123)) < 0) {
	return -1;
    }

    uint8_t data[48] = { 0x1b, };
    memset(data, 0, sizeof(data));
    data[0] = 0x1b;

    if (send(fd, data, sizeof(data), 0) == sizeof(data) &&
	recv(fd, data, sizeof(data), 0) == sizeof(data)) {
	uint32_t secs = sntp_value(&data[40]);
	uint32_t fractional = sntp_value(&data[44]);

	if (secs <= SNTP_SECS_AT_UNIX_EPOCH) {
	    errno = EINVAL;
	} else {
	    now->tv_sec = secs - SNTP_SECS_AT_UNIX_EPOCH;
	    now->tv_nsec = ((double) fractional) * 1.0e6 / NTP_SCALE_FRAC;
	    ret = 0;
	}
    }

    closesocket(fd);
    return ret;
}
