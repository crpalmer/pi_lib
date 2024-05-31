#include "pico/stdio/driver.h"
#include "pico/stdio_uart.h"
#include "uart-writer.h"

UartWriter::UartWriter() {
    stdio_set_driver_enabled(&stdio_uart, false);
}

int UartWriter::write_str(const char *s) {
    /* TODO: Look for the \n and write longer contiguous chunks of string */
    size_t n = 0;
    for (n = 0; s[n]; s++) {
	stdio_uart.out_chars(&s[n], 1);
	if (s[n] == '\n') stdio_uart.out_chars("\r", 1);
    }
    return n;
}
