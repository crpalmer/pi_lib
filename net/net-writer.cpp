#include "pi.h"
#include <string.h>
#include "net.h"
#include "net-writer.h"

NetWriter::NetWriter(int fd) {
    this->fd = fd;
}

int NetWriter::write_str(const char *line) {
    return send(fd, line, strlen(line), 0);
}
