#include "mem.h"
#include "net.h"
#include "net-reader.h"

NetReader::NetReader(int fd) {
    this->fd = fd;
    a = 128;
    n = 0;
    buf = (char *) fatal_malloc(a);
}

const char *NetReader::readline() {
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
