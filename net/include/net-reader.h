#ifndef __NET_READER_H__
#define __NET_READER_H__

#include "reader.h"

class NetReader : public Reader {
public:
    NetReader(int fd);
    const char *readline();

private:
    int fd;
    char *buf;
    size_t n, a;
};

#endif
