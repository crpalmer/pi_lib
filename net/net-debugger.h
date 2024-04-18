#ifndef __NET_DEBUGGER_H__
#define __NET_DEBUGGER_H__

#include "debugger.h"
#include "net.h"
#include "net-reader.h"
#include "net-writer.h"

class NetDebugger : public Debugger {
public:
    NetDebugger(Reader *r, Writer *w) : Debugger(r, w) {}
    NetDebugger(int fd) : Debugger(new NetReader(fd), new NetWriter(fd)) {}

    ~NetDebugger() {
	if (fd >= 0) close(fd);
    }

private:
    int fd = -1;
};

#endif
