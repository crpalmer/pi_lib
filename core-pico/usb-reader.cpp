#include "pico/stdio/driver.h"
#include "pico/stdio_usb.h"
#include "mem.h"
#include "usb-reader.h"

USBReader::USBReader(size_t buf_size) {
    a_buf = buf_size;
    buf = (char *) fatal_malloc(a_buf);
    stdio_set_driver_enabled(&stdio_usb, false);
}

const char *USBReader::readline() {
    size_t n_buf = 0;

    while (1) {
	char c;

	if (stdio_usb.in_chars(&c, 1) == 1) {
	    stdio_usb.out_chars(&c, 1);
	    if (c == '\r' || c == '\n') {
		buf[n_buf] = '\0';
		return buf;
	    } else if (c == 8) {
		if (n_buf) {
		    n_buf--;
		    stdio_usb.out_chars(" \b", 2);
		}
	    } else if (n_buf < a_buf-1) {
		buf[n_buf++] = c;
	    }
	}
    }
}
