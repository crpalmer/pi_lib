#ifndef __STDIN_USB_READER_H__
#define __STDIN_USB_READER_H__

#include <stdio.h>
#include "pico/stdio/driver.h"

#include "consoles.h"
#include "mem.h"
#include "pi.h"
#include "reader.h"

class StdioDriverReader : public Reader {
public:
    StdioDriverReader(stdio_driver_t *driver, size_t buf_size = 256) : driver(driver) {
	a_buf = buf_size;
	buf = (char *) fatal_malloc(a_buf);
	stdio_set_driver_enabled(driver, false);
    }

    const char *readline() override {
	size_t n_buf = 0;

	while (1) {
	    char c;

	    if (driver->in_chars(&c, 1) == 1) {
		driver->out_chars(&c, 1);
		if (c == '\r' || c == '\n') {
		    buf[n_buf] = '\0';
		    return buf;
		} else if (c == 8) {
		    if (n_buf) {
			n_buf--;
			driver->out_chars(" \b", 2);
		    }
		} else if (n_buf < a_buf-1) {
		    buf[n_buf++] = c;
		}
	    }
	}
    }

private:
    stdio_driver_t *driver;
    char  *buf;
    size_t a_buf;
};

#endif
