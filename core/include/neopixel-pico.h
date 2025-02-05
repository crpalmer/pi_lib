#ifndef __NEOPIXEL_PICO_H__
#define __NEOPIXEL_PICO_H__

typedef enum {
    neopixel_mode_RGB,
    neopixel_mode_GRB
} neopixel_mode_t;

typedef struct {
    unsigned char r, b, g;
} neopixel_rgb_t;

class NeoPixelPico {
public:
    NeoPixelPico(int pin, bool use_gamma = true);

    void set_mode(neopixel_mode_t new_mode);
    void set_n_leds(int new_n_leds);

    void set_all(unsigned char r, unsigned char g, unsigned char b);
    void set_led(int led, unsigned char r, unsigned char g, unsigned char b);
    void set_led(int led, unsigned char v) { set_led(led, v, v, v); }

    void set_brightness(double pct);

    void show();

    int get_n_leds() { return n_leds; }
    neopixel_rgb_t get_led(int led) {
	if (led < n_leds) return leds[led];
	else {
	    neopixel_rgb_t zero = { 0, 0, 0};
	    return zero;
	}
    }

private:
    int pio;
    int sm;
    int n_leds;
    bool use_gamma;
    neopixel_rgb_t *leds;
    neopixel_mode_t mode;
    double brightness;

    unsigned pio_data_raw(int led, int byte);
    unsigned pio_data(int led, int byte);
};

#endif
