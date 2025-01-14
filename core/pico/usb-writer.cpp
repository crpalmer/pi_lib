#include "pico/stdio/driver.h"
#include "pico/stdio_usb.h"
#include "usb-writer.h"

USBWriter::USBWriter() {
    stdio_set_driver_enabled(&stdio_usb, false);
}

int USBWriter::write_str(const char *s) {
    /* TODO: Look for the \n and write longer contiguous chunks of string */
    size_t n = 0;
    for (n = 0; s[n]; s++) {
	stdio_usb.out_chars(&s[n], 1);
	if (s[n] == '\n') stdio_usb.out_chars("\r", 1);
    }
    return n;
}
