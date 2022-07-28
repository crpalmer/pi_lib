#include <stdio.h>
#include <string.h>
#include "neopixel-pi.h"
#include "pi-usb.h"
#include "util.h"

#define VENDOR_ID 0x2e8a

NeoPixelPI::NeoPixelPI() {
    tty = -1;
}

void
NeoPixelPI::reboot()
{
    writeline("bootsel");
}

void
NeoPixelPI::writeline(const char *l)
{
    if (l == NULL) l = line;

    while (! ensure_tty()) {
	ms_sleep(1000);
    }

    write(tty, l, strlen(l));
    write(tty, "\n", 1);
}

bool
NeoPixelPI::ensure_tty()
{
    if (tty < 0) {
	tty = pi_usb_open_tty(VENDOR_ID, 0);
	if (tty < 0) return false;
	writeline("");
    }
    return true;
}

void NeoPixelPI::set_n_leds(int n_leds)
{
    sprintf(line, "set_n_leds %d\n", n_leds);
    writeline();
}

void NeoPixelPI::set_led(int led, unsigned char r, unsigned char g, unsigned char b)
{
    sprintf(line, "set_led %d %d %d %d\n", led, r, g, b);
    writeline();
}

void NeoPixelPI::show()
{
    writeline("show");
}

void NeoPixelPI::reboot_bootsel()
{
    writeline("bootsel");
}

void NeoPixelPI::set_brightness(double brightness)
{
    sprintf(line, "set_brightness %f", brightness);
    writeline();
}

void NeoPixelPI::set_all(unsigned char r, unsigned char g, unsigned char b)
{
    sprintf(line, "set_all %d %d %d", r, g, b);
    writeline();
}
