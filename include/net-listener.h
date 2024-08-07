#ifndef __NET_LISTENER_H__
#define __NET_LISTENER_H__

#include <stdint.h>
#include "pi-threads.h"

class NetListener : public PiThread {
public:
    NetListener(uint16_t port = 4567, const char *name = "net-listener") : PiThread(name), port(port) { }
    virtual void accepted(int fd) = 0;
    void main();

private:
    uint16_t port;
};

#endif
