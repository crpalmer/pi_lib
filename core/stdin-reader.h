#ifndef __STDIN_READER_H__
#define __STDIN_READER_H__

#include <stdio.h>
#include "pi.h"
#include "mem.h"
#include "reader.h"

class StdinReader : public Reader {
public:
    StdinReader(size_t buf_size = 256) {
	a_buf = buf_size;
	buf = (char *) fatal_malloc(a_buf);
    }

    const char *readline() override {
	return pi_readline(buf, a_buf);
    }

private:
    char  *buf;
    size_t a_buf;
};

#endif
