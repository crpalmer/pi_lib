#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "hardware/pio.h"
#include "mem.h"

#include "neopixel-pico.h"
#include "neopixel.pio.h"

static inline PIO PIOx(int pio) { return pio == 0 ? pio0 : pio1; }

const uint8_t gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

NeoPixelPico::NeoPixelPico(int pin, bool use_gamma)
{
    n_leds = 0;
    this->use_gamma = use_gamma;
    leds = (neopixel_rgb_t *) fatal_malloc(1);
    mode = neopixel_mode_GRB;
    brightness = 1;

    if ((sm = pio_claim_unused_sm(pio0, false)) >= 0) {
        pio = 0;
    } else if ((sm = pio_claim_unused_sm(pio1, false)) >= 0) {
        pio = 1;
    } else {
        fprintf(stderr, "All PIO slots are full.  Fatal.\n");
        exit(1);
    }

    neopixel_program_init(PIOx(pio), sm, pio_add_program(PIOx(pio), &neopixel_program), pin, 800*1000, 8);
}

void NeoPixelPico::set_mode(neopixel_mode_t new_mode)
{
    mode = new_mode;
}

unsigned NeoPixelPico::pio_data_raw(int led, int byte)
{
    if (led >= n_leds) return 0;
    if (mode == neopixel_mode_RGB) {
	switch(byte) {
	case 0: return leds[led].r;
	case 1: return leds[led].g;
	case 2: return leds[led].b;
	}
    } else if (mode == neopixel_mode_GRB) {
	switch(byte) {
	case 0: return leds[led].g;
	case 1: return leds[led].r;
	case 2: return leds[led].b;
	}
    }
    return 0;
}

unsigned NeoPixelPico::pio_data(int led, int byte)
{
    unsigned v = pio_data_raw(led, byte);
    v = v * brightness + 0.5;
    if (v > 255) v = 255;
    if (use_gamma) v = gamma8[v];
    return v;
}

void NeoPixelPico::set_n_leds(int new_n_leds)
{
    if (n_leds >= new_n_leds) {
	n_leds = new_n_leds;
	return;
    }

    leds = (neopixel_rgb_t *) fatal_realloc(leds, sizeof(*leds) * new_n_leds);

    while (n_leds < new_n_leds) {
	leds[n_leds].r = leds[n_leds].g = leds[n_leds].b = 0;
	n_leds++;
    }
}

void NeoPixelPico::set_all(unsigned char r, unsigned char g, unsigned char b)
{
    for (int led = 0; led < n_leds; led++) {
	set_led(led, r, g, b);
    }
}

void NeoPixelPico::set_led(int led, unsigned char r, unsigned char g, unsigned char b)
{
    if (led < n_leds) {
	leds[led].r = r;
	leds[led].g = g;
	leds[led].b = b;
    }
}

void NeoPixelPico::set_brightness(double brightness)
{
    this->brightness = brightness;
}

void NeoPixelPico::show()
{
    for (int led = 0; led < n_leds; led++) {
	for (int byte = 0; byte < 3; byte++) {
	    unsigned value = pio_data(led, byte);
	    pio_sm_put_blocking(PIOx(pio), sm, value << 24);
	}
    }
}
