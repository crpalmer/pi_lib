#include "pico/stdio/driver.h"
#include "pico/stdio_uart.h"
#include "mem.h"
#include "pi.h"
#include "uart-reader.h"

UartReader::UartReader(size_t buf_size) {
    a_buf = buf_size;
    buf = (char *) fatal_malloc(a_buf);
    stdio_set_driver_enabled(&stdio_uart, false);
}

const char *UartReader::readline() {
    size_t n_buf = 0;

    while (1) {
	char c;

	if (stdio_uart.in_chars(&c, 1) == 1) {
	    stdio_uart.out_chars(&c, 1);
	    if (c == '\r' || c == '\n') {
		buf[n_buf] = '\0';
		return buf;
	    } else if (c == 8) {
		if (n_buf) {
		    n_buf--;
		    stdio_uart.out_chars(" \b", 2);
		}
	    } else if (n_buf < a_buf-1) {
		buf[n_buf++] = c;
	    }
	}
    }
}
