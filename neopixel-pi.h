#ifndef __NEOPIXEL_PI_H__
#define __NEOPIXEL_PI_H__

#include "pico-slave.h"

class NeoPixelPI {
public:
    NeoPixelPI();
    void reboot();

    void set_n_leds(int n_leds);

    void set_all(unsigned char r, unsigned char g, unsigned char b);
    void set_led(int led, unsigned char r, unsigned char g, unsigned char b);

    void show();

    void set_brightness(double brightness);

    void reboot_bootsel();

private:
    PicoSlave *pico;
    char line[10*1024];
};

#endif
