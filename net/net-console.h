#ifndef __NET_CONSOLE_H__
#define __NET_CONSOLE_H__

#include "threads-console.h"
#include "net.h"
#include "net-reader.h"
#include "net-writer.h"

class NetConsole : public ThreadsConsole {
public:
    NetConsole(Reader *r, Writer *w) : ThreadsConsole(r, w) { }
    NetConsole(int fd) : ThreadsConsole(new NetReader(fd), new NetWriter(fd)) { }

    ~NetConsole() {
	if (fd >= 0) closesocket(fd);
    }

private:
    int fd = -1;
};

#endif
