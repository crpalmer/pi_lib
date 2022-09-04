#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include "hardware/pio.h"
#include "mem.h"

#include "neopixel-pico.h"
#include "neopixel.pio.h"

NeoPixelPico::NeoPixelPico(int pin)
{
    n_leds = 0;
    leds = (neopixel_rgb_t *) fatal_malloc(1);
    mode = neopixel_mode_GRB;
    brightness = 1;

    if ((sm = pio_claim_unused_sm(pio0, false)) >= 0) {
        pio = pio0;
    } else if ((sm = pio_claim_unused_sm(pio1, false)) >= 0) {
        pio = pio1;
    } else {
        fprintf(stderr, "All PIO slots are full.  Fatal.\n");
        exit(1);
    }

    neopixel_program_init(pio, sm, pio_add_program(pio, &neopixel_program), pin, 800*1000, 8);
}

unsigned NeoPixelPico::pio_data(int led, int byte)
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
	    unsigned value = pio_data(led, byte) * brightness + 0.5;
	    pio_sm_put_blocking(pio, sm, value << 24);
	}
    }
}
