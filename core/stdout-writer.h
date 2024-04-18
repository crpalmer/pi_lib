#ifndef __STDOUT_WRITER_H__
#define __STDOUT_WRITER_H__

#include "writer.h"

class StdoutWriter : public Writer {
public:
    int writeline(const char *s) override { return puts(s); }
};

#endif
