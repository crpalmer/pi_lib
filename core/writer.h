#ifndef __WRITER_H__
#define __WRITER_H__

#include <stdio.h>
#include <stdarg.h>
#include "mem.h"

class Writer {
public:
    Writer() {
	have = 128;
	buf = (char *) fatal_malloc(have);
    }

    virtual int write_str(const char *str) = 0;

    int printf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int ret = printf_va(fmt, ap);
        va_end(ap);

	return ret;
    }

    int printf_va(const char *fmt, va_list va) {
	va_list new_va;
	va_copy(new_va, va);
 	size_t need = vsnprintf(buf, have, fmt, va);
	va_end(new_va);

	if (need+1 >= have) {
	    have = (need + 1) * 2;
	    buf = (char *) fatal_realloc(buf, have);

	    va_copy(new_va, va);
	    vsnprintf(buf, have, fmt, va);
	    va_end(new_va);
	}

	return write_str(buf);
    }

private:
    size_t have;
    char *buf;
};

#endif
