#ifndef __NET_WRITER_H__
#define __NET_WRITER_H__

#include "net.h"
#include "writer.h"

class NetWriter : public Writer {
public:
    NetWriter(int fd) {
	this->fd = fd;
    }

    int write_str(const char *line) { return send(fd, line, strlen(line), 0); }

private:
    int fd;
};

#endif
