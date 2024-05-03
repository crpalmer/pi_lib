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

static void report_time()
{
    struct tm *tm_info, tm_space;
    char buf[100];

    time_t t = time(NULL);
    tm_info = localtime_r(&t, &tm_space);

    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    consoles_printf("Local time: %s\n", buf);
}

class ConsoleThread : public NetConsole, public PiThread {
public:
    ConsoleThread(Reader *r, Writer *w) : NetConsole(r, w), PiThread("console") { start(); }
    ConsoleThread(int fd) : NetConsole(fd), PiThread("console") { start(); }

    void main() { Console::main(); }
    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "run-sntp")) {
	    net_sntp_set_pico_rtc(NULL);
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
   SNTPThread() : PiThread("sntp") { start(); }

    void main() {
        while (1) {
	    net_sntp_set_pico_rtc(NULL);
	    ms_sleep(60*60*1000);
       }
    }
};

class ReportingThread : PiThread {
public:
    ReportingThread() : PiThread("time-reporting") { start(); }

    void main() {
	while (1) {
	    report_time();
	    ms_sleep(60*1000);
	}
    }
};

class NetThread : NetListener {
public:
    NetThread(uint16_t port) : NetListener(port) { start(); }

    void accepted(int fd) {
	new ConsoleThread(fd);
    }
};

static void
threads_main(int argc, char **argv)
{
    new ConsoleThread(new StdinReader(), new StdoutWriter());
    wifi_init();
    wifi_wait_for_connection();
    new NetThread(4567);
    new SNTPThread();
    new ReportingThread();
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
