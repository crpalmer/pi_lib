#ifndef __THREADS_CONSOLE_H__
#define __THREADS_CONSOLE_H__

#include "pi-threads.h"
#include "consoles.h"

class ThreadsConsole : public Console {
public:
    ThreadsConsole(Reader *r, Writer *w) : Console(r, w) {}

    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "threads")) {
	    char *state = pi_threads_get_state();
	    consoles_write_str(state);
	    free(state);
	} else {
	    Console::process_cmd(cmd);
	}
    }

    void usage() override {
	Console::usage();
	write_str("threads - dump the current thread state\n");
    }
};

#endif
