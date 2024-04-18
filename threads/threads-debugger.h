#ifndef __THREADS_DEBUGGER_H__
#define __THREADS_DEBUGGER_H__

#include "net-debugger.h"
#include "pi-threads.h"

class ThreadsDebugger : public NetDebugger {
public:
    ThreadsDebugger(Reader *r, Writer *w) : NetDebugger(r, w) {}
    ThreadsDebugger(int fd) : NetDebugger(fd) {}

    void process_cmd(const char *cmd) override {
	if (is_command(cmd, "threads")) {
	    char *state = pi_threads_get_state();
	    writeline(state);
	    free(state);
	} else {
	    NetDebugger::process_cmd(cmd);
	}
    }

    void usage() override {
	NetDebugger::usage();
	writeline("threads - dump the current thread state\n");
    }
};

#endif
