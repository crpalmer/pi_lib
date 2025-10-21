#ifndef __STDOUT_WRITER_H__
#define __STDOUT_WRITER_H__

#include <string.h>
#include "writer.h"

class StdoutWriter : public Writer {
public:
    int write_str(const char *s) override { return fwrite(s, 1, strlen(s), stdout); }
};

#endif
