#ifndef __DEBUGGER_H__
#define __DEBUGGER_H__

#include <string.h>
#include "reader.h"
#include "writer.h"

class Debugger {
public:
    Debugger() {}

    Debugger(Reader *reader, Writer *writer) {
	r = reader;
	w = writer;
    }

    void run() {
	const char *cmd;

	printf("Debugger is running\n");
	while ((cmd = r->readline()) != NULL) {
	    process_cmd(cmd);
	}
    }

    virtual void process_cmd(const char *cmd) {
	size_t n = strlen(cmd);
	if (is_command(cmd, "bootsel")) {
	    pi_reboot_bootloader();
	} else if (is_command(cmd, "?") || is_command(cmd, "help")) {
	    usage();
	} else {
	    w->writeline("Invalid command, type ? or help for help.\n");
	}
    }

    virtual void usage() {
	w->writeline("Usage:\n\nbootsel - reboot to bootloader.\n");
    }

    bool is_command(const char *input, const char *cmd) {
	size_t n = strlen(cmd);
	return strncmp(input, cmd, n) == 0 && (cmd[n] == '\0' || cmd[n] == ' ');
    }

    void writeline(const char *str) { w->writeline(str); }
    const char *readline() { return r->readline(); }

private:
    Reader *r;
    Writer *w;
};

#endif
