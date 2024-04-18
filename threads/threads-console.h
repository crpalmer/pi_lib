#ifndef __THREADS_CONSOLE_H__
#define __THREADS_CONSOLE_H__

#include "consoles.h"
#include "net-console.h"
#include "pi-threads.h"

class ThreadsConsole : public NetConsole {
public:
    ThreadsConsole(Reader *r, Writer *w) : NetConsole(r, w) {}
    ThreadsConsole(int fd) : NetConsole(fd) {}

    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "threads")) {
	    char *state = pi_threads_get_state();
	    consoles_write_str(state);
	    free(state);
	} else {
	    NetConsole::process_cmd(cmd);
	}
    }

    void usage() override {
	NetConsole::usage();
	consoles_write_str("threads - dump the current thread state\n");
    }
};

#endif
