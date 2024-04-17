#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "net.h"
#include "pi-threads.h"
#include "wifi.h"
#include "sntp.h"

#include "time-utils.h"

static void
sntp_main(void *unused)
{
    while (1) {
	struct timespec now;

	while (net_sntp_time(NULL, &now) < 0) {
	    perror("net_sntp_time");
	    ms_sleep(5*1000);
	}

	printf("Setting RTC to %lu seconds.\n", (unsigned long) now.tv_sec);
	pico_set_rtc((time_t) now.tv_sec);
	ms_sleep(1*1000);
   }
}

static void
wifi_main(void *unused)
{
    pi_thread_create("sntp", sntp_main, NULL);

    while (1) {
	char buffer[100];
	struct tm *tm_info, tm_space;

	time_t t = time(NULL);
	tm_info = localtime_r(&t, &tm_space);

	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
	printf("local time: %s [%lu]\n", buffer, (unsigned long) t);

	ms_sleep(1*1000);
    }
}

int
main(int argc, char **argv)
{
    pi_init_with_wifi(wifi_main, NULL);
}
