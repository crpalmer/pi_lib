#ifndef __STDOUT_USB_WRITER_H__
#define __STDOUT_USB_WRITER_H__

#include <string.h>
#include "pico/stdio/driver.h"
#include "writer.h"

class StdioDriverWriter : public Writer {
public:
    StdioDriverWriter(stdio_driver_t *driver) : driver(driver) {
        stdio_set_driver_enabled(driver, false);
    }

    int write_str(const char *s) override {
	/* TODO: Look for the \n and write longer contiguous chunks of string */
	size_t n = 0;
	for (n = 0; s[n]; s++) {
	    driver->out_chars(&s[n], 1);
	    if (s[n] == '\n') driver->out_chars("\r", 1);
	}
	return n;
    }

private:
    stdio_driver_t *driver;
};

#endif
