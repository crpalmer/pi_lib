#include <stdio.h>
#include <string.h>
#include "neopixel-pi.h"
#include "pi-usb.h"
#include "util.h"

#define VENDOR_ID 0x2e8a

PicoSlave::PicoSlave() {
    tty = -1;
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
	writeline("");
    }
    return true;
}
