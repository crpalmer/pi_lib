#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "net.h"
#include "net-console.h"
#include "net-listener.h"
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

class ConsoleThread : public NetConsole {
public:
    ConsoleThread(Reader *r, Writer *w) : NetConsole(r, w) { }
    ConsoleThread(int fd) : NetConsole(fd) { }

    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "run-sntp")) {
	    run_sntp();
	} else if (is_command(cmd, "get-time")) {
	    report_time();
	} else {
	    NetConsole::process_cmd(cmd);
	}
    }

    void usage() override {
	NetConsole::usage();
	write_str("run-sntp - Start an sntp query\nget-time - get the current local time\n");
    }
};

class SNTPThread : PiThread {
public:
   SNTPThread() : PiThread("sntp") { }

    void main() {
        while (1) {
	    run_sntp();
	    ms_sleep(60*60*1000);
       }
    }
};

class ReportingThread : PiThread {
public:
    ReportingThread() : PiThread("time-reporting") {}

    void main() {
	while (1) {
	    report_time();
	    ms_sleep(60*1000);
	}
    }
};

class NetThread : NetListener {
public:
    NetThread(uint16_t port) : NetListener(port) { }

    void accepted(int fd) {
	new ConsoleThread(fd);
    }
};

static void
wifi_main(void *unused)
{
    new ConsoleThread(new StdinReader(), new StdoutWriter());
    new SNTPThread();
    new NetThread(4567);
    new ReportingThread();
}

int
main(int argc, char **argv)
{
    pi_init_with_wifi(wifi_main, NULL);
}
