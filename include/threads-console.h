#ifndef __THREADS_CONSOLE_H__
#define __THREADS_CONSOLE_H__

#include "pi-threads.h"
#include "consoles.h"

class ThreadsConsole : public Console {
public:
    ThreadsConsole(Reader *r, Writer *w) : Console(r, w) {}

    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "threads")) {
	    pi_threads_dump_state();
	} else if (is_command(cmd, "free")) {
	    printf("Free RAM: %ld\n", (long) pi_threads_get_free_ram());
	} else {
	    Console::process_cmd(cmd);
	}
    }

    void usage() override {
	Console::usage();
	printf("free - report amount of free memory\nthreads - dump the current thread state\n");
    }
};

#endif
