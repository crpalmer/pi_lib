#ifndef __NEOPIXEL_PI_H__
#define __NEOPIXEL_PI_H__

class NeoPixelPI {
public:
    NeoPixelPI();
    void reboot();

    void set_n_leds(int n_leds);
    void set_led(int led, unsigned char r, unsigned char g, unsigned char b);
    void show();

    void reboot_bootsel();

private:
    void writeline(const char *l = NULL);
    bool ensure_tty(void);

    int tty;
    char line[10*1024];
};

#endif
