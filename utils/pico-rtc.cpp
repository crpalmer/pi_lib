#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "threads-debugger.h"
#include "net.h"
#include "pi-threads.h"
#include "stdin-reader.h"
#include "stdout-writer.h"
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

    printf("Setting RTC to %lu seconds.\n", (unsigned long) now.tv_sec);
    pico_set_rtc((time_t) now.tv_sec);
}

static void gettime(char *buf, size_t n_buf)
{
    struct tm *tm_info, tm_space;

    time_t t = time(NULL);
    tm_info = localtime_r(&t, &tm_space);

    strftime(buf, n_buf, "%Y-%m-%d %H:%M:%S", tm_info);
}

class SNTPDebugger : public ThreadsDebugger {
public:
    SNTPDebugger(Reader *r, Writer *w) : ThreadsDebugger(r, w) {}
    SNTPDebugger(int fd) : ThreadsDebugger(fd) {}

    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "run-sntp")) {
	    run_sntp();
	} else if (is_command(cmd, "get-time")) {
	    char buf[100];
	    gettime(buf, sizeof(buf));
	    writeline(buf);
	} else {
	    ThreadsDebugger::process_cmd(cmd);
	}
    }

    void usage() override {
	ThreadsDebugger::usage();
	writeline("run-sntp - Start an sntp query\nget-time - get the current local time\n");
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
stdio_main(void *unused)
{
   SNTPDebugger *d = new SNTPDebugger(new StdinReader(), new StdoutWriter());
   d->run();
}

static void
run_net_debug(void *debug_as_vp)
{
    SNTPDebugger *debug = (SNTPDebugger *) debug_as_vp;

    debug->run();
}

static void
net_main(void *unused)
{
    while (1) {
	int l_fd = net_listen(4567);

	while (1) {
	    int fd;
	    struct sockaddr_in client;
	    socklen_t size = sizeof(client);

	    if ((fd = accept(l_fd, (struct sockaddr *) &client, &size)) < 0) {
	 	perror("net_accept");
		ms_sleep(5*1000);
	    } else {
		fprintf(stderr,
		    "connect from host %s, port %hu.\n",
		    inet_ntoa(client.sin_addr),
		    ntohs(client.sin_port)
		);

		pi_thread_create("net-debug", run_net_debug, new SNTPDebugger(fd));
	    }
	}
    }
}

static void
wifi_main(void *unused)
{
    pi_thread_create("sntp", sntp_main, NULL);
    pi_thread_create("stdio", stdio_main, NULL);
    pi_thread_create("net", net_main, NULL);

    while (1) {
	char buffer[100];
	gettime(buffer, sizeof(buffer));
	printf("local time: %s\n", buffer);
	ms_sleep(60*1000);
    }
}

int
main(int argc, char **argv)
{
    pi_init_with_wifi(wifi_main, NULL);
}
