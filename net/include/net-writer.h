#ifndef __NET_WRITER_H__
#define __NET_WRITER_H__

#include "writer.h"

class NetWriter : public Writer {
public:
    NetWriter(int fd);
    int write_str(const char *line);

private:
    int fd;
};

#endif
