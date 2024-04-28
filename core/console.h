#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <string.h>
#include "consoles.h"
#include "reader.h"
#include "writer.h"

class Console : public Writer {
public:
    Console() : Writer() {
	consoles_add(this);
    }

    Console(Reader *reader, Writer *writer) : Writer(), r(reader), w(writer) {
	consoles_add(this);
    }

    ~Console() {
	consoles_remove(this);
    }

    void main() {
	const char *cmd;

	while ((cmd = r->readline()) != NULL) {
	    process_cmd(cmd);
	}
    }

    virtual void process_cmd(const char *cmd);
    virtual void usage();

    bool is_command(const char *input, const char *cmd) {
	size_t n = strlen(cmd);
	return strncmp(input, cmd, n) == 0 && (cmd[n] == '\0' || cmd[n] == ' ');
    }

    virtual int write_str(const char *str) {
	return w ? w->write_str(str) : -1;
    }

private:
    Reader *r;
    Writer *w;
};

#endif
