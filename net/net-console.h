#ifndef __NET_CONSOLE_H__
#define __NET_CONSOLE_H__

#include "console.h"
#include "net.h"
#include "net-reader.h"
#include "net-writer.h"

class NetConsole : public Console {
public:
    NetConsole(Reader *r, Writer *w) : Console(r, w) {}
    NetConsole(int fd) : Console(new NetReader(fd), new NetWriter(fd)) {}

    ~NetConsole() {
	if (fd >= 0) close(fd);
    }

private:
    int fd = -1;
};

#endif
