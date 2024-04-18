#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "threads-console.h"
#include "net.h"
#include "pi-threads.h"
#include "stdin-reader.h"
#include "stdout-writer.h"
#include "consoles.h"
#include "wifi.h"
#include "sntp.h"

#include "time-utils.h"

static void run_sntp()
{
    struct timespec now;

    while (net_sntp_time(NULL, &now) < 0) {
	perror("net_sntp_time");
	ms_sleep(5*1000);
    }

    consoles_printf("Setting RTC to %lu seconds.\n", (unsigned long) now.tv_sec);
    pico_set_rtc((time_t) now.tv_sec);
}

static void report_time()
{
    struct tm *tm_info, tm_space;
    char buf[100];

    time_t t = time(NULL);
    tm_info = localtime_r(&t, &tm_space);

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    consoles_printf("Local time: %s\n", buf);
}

class RTCConsole : public ThreadsConsole {
public:
    RTCConsole(Reader *r, Writer *w) : ThreadsConsole(r, w) { }
    RTCConsole(int fd) : ThreadsConsole(fd) { }

    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "run-sntp")) {
	    run_sntp();
	} else if (is_command(cmd, "get-time")) {
	    report_time();
	} else {
	    ThreadsConsole::process_cmd(cmd);
	}
    }

    void usage() override {
	ThreadsConsole::usage();
	consoles_write_str("run-sntp - Start an sntp query\nget-time - get the current local time\n");
    }
};

static void
sntp_main(void *unused)
{
    while (1) {
	run_sntp();
	ms_sleep(60*60*1000);
   }
}

static void
console_main(void *console_as_vp)
{
    RTCConsole *console = (RTCConsole *) console_as_vp;

    consoles_add(console);
    console->run();
}

static void
net_main(void *unused)
{
    consoles_write_str("net: Starting.\n");

    while (1) {
 	uint16_t port = 4567;

	consoles_printf("net: Listening on port %u\n", port);
	int l_fd = net_listen(port);

	while (1) {
	    int fd;
	    struct sockaddr_in client;
	    socklen_t size = sizeof(client);

	    consoles_write_str("net: Waiting for connection.\n");
	    if ((fd = accept(l_fd, (struct sockaddr *) &client, &size)) < 0) {
	 	perror("net_accept");
		ms_sleep(5*1000);
	    } else {
		consoles_printf("connect from host %s, port %hu.\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		pi_thread_create("net-debug", console_main, new RTCConsole(fd));
	    }
	}
    }
}

static void
wifi_main(void *unused)
{
    pi_thread_create("stdio", console_main, new RTCConsole(new StdinReader(), new StdoutWriter()));
    consoles_write_str("Starting SNTP thread.\n");
    pi_thread_create("sntp", sntp_main, NULL);
    consoles_write_str("Starting net thread.\n");
    pi_thread_create("net", net_main, NULL);

    consoles_write_str("Entering main loop.\n");
    while (1) {
	report_time();
	ms_sleep(60*1000);
    }
}

int
main(int argc, char **argv)
{
    pi_init_with_wifi(wifi_main, NULL);
}
