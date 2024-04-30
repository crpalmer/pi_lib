#include <stdio.h>
#include <string.h>
#include "neopixel-pi.h"
#include "pi-usb.h"

#define VENDOR_ID 0x2e8a

NeoPixelPI::NeoPixelPI() {
    pico = new PicoSlave();
}

void
NeoPixelPI::reboot()
{
    pico->writeline("bootsel");
}

void NeoPixelPI::set_n_leds(int n_leds)
{
    sprintf(line, "set_n_leds %d\n", n_leds);
    pico->writeline(line);
}

void NeoPixelPI::set_led(int led, unsigned char r, unsigned char g, unsigned char b)
{
    sprintf(line, "set_led %d %d %d %d\n", led, r, g, b);
    pico->writeline(line);
}

void NeoPixelPI::show()
{
    pico->writeline("show");
}

void NeoPixelPI::reboot_bootsel()
{
    pico->writeline("bootsel");
}

void NeoPixelPI::set_brightness(double brightness)
{
    sprintf(line, "set_brightness %f", brightness);
    pico->writeline(line);
}

void NeoPixelPI::set_all(unsigned char r, unsigned char g, unsigned char b)
{
    sprintf(line, "set_all %d %d %d", r, g, b);
    pico->writeline(line);
}
