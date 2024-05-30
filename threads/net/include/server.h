#ifndef __SERVER_H__
#define __SERVER_H__

#include "mem.h"
#include "net-reader.h"
#include "net-writer.h"
#include "pi-threads.h"

class ServerConnection : public PiThread {
public:
    ServerConnection(NetReader *r, NetWriter *w, const char *name = "server") : PiThread(name), r(r), w(w) { }

    void main() override {
	const char *line;
	while ((line = r->readline()) != NULL) {
	    char *response = process_cmd(line);
	    for (size_t i = 0; response[i]; i++) if (response[i] == '\n') response[i] = ' ';
	    w->printf("%s\n", response);
	    fatal_free(response);
	}
    }

    virtual char *process_cmd(const char *cmd) = 0;

private:
    Reader *r;
    Writer *w;
};

#endif
