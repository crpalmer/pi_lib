#ifndef __NET_READER_H__
#define __NET_READER_H__

#include "mem.h"
#include "net.h"
#include "reader.h"

class NetReader : public Reader {
public:
    NetReader(int fd) {
	this->fd = fd;
	a = 128;
	n = 0;
	buf = (char *) fatal_malloc(a);
    }

    const char *readline() {
	while (1) {
	    if (n >= a-1) {
		a *= 2;
		buf = (char *) fatal_realloc(buf, a);
	    }
	    if (recv(fd, &buf[n], 1, 0) != 1) return NULL;
	    if (buf[n] == '\n') {
		buf[n] = '\0';
		n = 0;
		return buf;
	    } else if (buf[n] != '\r') {
		n++;
	    }
	}
    }

private:
    int fd;
    char *buf;
    size_t n, a;
};

#endif
