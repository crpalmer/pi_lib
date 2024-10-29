#include <stdio.h>
#include <string.h>
#include "neopixel-pi.h"
#include "pi-usb.h"
#include "time-utils.h"

#define VENDOR_ID 0x2e8a

PicoSlave::PicoSlave() {
    tty = -1;
    ensure_tty();
}

void
PicoSlave::writeline(const char *l)
{
    while (! ensure_tty()) {
	perror("Could not find the pico");
	ms_sleep(1000);
    }

    write(tty, l, strlen(l));
    write(tty, "\n", 1);
}

bool
PicoSlave::ensure_tty()
{
    if (tty < 0) {
	tty = pi_usb_open_tty(VENDOR_ID, 0);
	if (tty < 0) return false;
    }
    return true;
}

bool
PicoSlave::readline(char *buf, int len)
{
    if (tty < 0) {
	fprintf(stderr, "PicoSlave: not connected.\n");
	return false;
    }

    int i;
    for (i = 0; i < len - 1; i++) {
	if (read(tty, &buf[i], 1) != 1) {
	    perror("read");
	    close(tty);
	    return false;
	}
	if (buf[i] == '\n' || buf[i] == '\r') break;
    }

    buf[i] = '\0';

    return true;
}
