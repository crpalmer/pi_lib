#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tusb.h>
#include "neopixel-pico.h"
#include "pi.h"
#include "time-utils.h"
#include "util.h"

static char line[100*1024];

#define STRNCMP(a, b) strncmp(a, b, strlen(b))

#define FADE_INC	1
#define FADE_SLEEP_MS	10

static bool
get_rgb(const char *usage, const char *line, int *r, int *g, int *b)
{
    if (sscanf(line, "%d %d %d", r, g, b) == 3) {
	return true;
    }
    printf("%s <r> <g> <b>\n", usage);
    return false;
}

static void
show(NeoPixelPico *neo)
{
    struct timespec start;

    nano_gettime(&start);
    neo->show();
    printf("show took %d ms\n", nano_elapsed_ms_now(&start));
}

static void
fade_in(NeoPixelPico *neo, int final_r, int final_g, int final_b)
{
    int r = 0, g = 0, b = 0;

    while (r < final_r || g < final_g || b < final_b) {
	if (r < final_r) r += FADE_INC;
	if (g < final_g) g += FADE_INC;
	if (b < final_b) b += FADE_INC;
	neo->set_all(r, g, b);
	neo->show();
	ms_sleep(FADE_SLEEP_MS);
    }
    printf("faded in to %d,%d,%d\n", r, g, b);
}

static void
fade_out(NeoPixelPico *neo, int r, int g, int b)
{
    while (r > 0 || g > 0 || b > 0) {
	if (r > 0) r -= FADE_INC;
	if (g > 0) g -= FADE_INC;
	if (b > 0) b -= FADE_INC;
	neo->set_all(r, g, b);
	neo->show();
	ms_sleep(FADE_SLEEP_MS);
    }
    printf("fade out complete.\n");
}

int
main()
{
    int n_leds = 0;

    pi_init_no_reboot();

    NeoPixelPico *neo = new NeoPixelPico(0);

    for (;;) {
	int r, g, b;

	while (!tud_cdc_connected()) {
	    ms_sleep(1);
	}

	pi_readline(line, sizeof(line));

	int space = 0;
	while (line[space] && line[space] != ' ') space++;

	if (strcmp(line, "bootsel") == 0) {
	    pi_reboot_bootloader();
	} else if (strcmp(line, "dump") == 0) {
	    for (int i = 0; i < neo->get_n_leds(); i++) {
		neopixel_rgb_t rgb = neo->get_led(i);
		printf("%3d %02x %02x %02x\n", i, rgb.r, rgb.g, rgb.b);
	    }
	} else if (strcmp(line, "help") == 0) {
	    printf("bootsel: reboot into bootloader mode\n");
	    printf("dump\n");
	    printf("fade_in <r> <g> <b>\n");
	    printf("fade_out <r> <g> <b>\n");
	    printf("set_all <r> <g> <b>\n");
	    printf("set_brightness <pct>\n");
	    printf("set_led <led> <r> <g> <b>\n");
	    printf("set_n_leds <n>\n");
	    printf("show\n");
	} else if (STRNCMP(line, "set_n_leds ") == 0) {
	    n_leds = atoi(&line[space]);
	    neo->set_n_leds(n_leds);
	} else if (n_leds == 0) {
	    printf("error: you must set_n_leds <n> first\n");
	} else if (STRNCMP(line, "set_brightness ") == 0) {
	    neo->set_brightness(atof(&line[space]));
	} else if (STRNCMP(line, "fade_in ") == 0) {
	    if (get_rgb("fade_in: usage", &line[space], &r, &g, &b)) {
		fade_in(neo, r, g, b);
	    }
	} else if (STRNCMP(line, "fade_out ") == 0) {
	    if (get_rgb("fade_in: usage", &line[space], &r, &g, &b)) {
		fade_out(neo, r, g, b);
	    }
	} else if (STRNCMP(line, "set_all ") == 0) {
	    if (get_rgb("set_all: usage", &line[space], &r, &g, &b)) {
	        neo->set_all(r, g, b);
		show(neo);
		printf("set_all to %d,%d,%d\n", r, g, b);
	    }
	} else if (STRNCMP(line, "set_led ") == 0) {
	    int led, r, g, b;

	    if (sscanf(&line[space], "%d %d %d %d", &led, &r, &g, &b) == 4) {
	        neo->set_led(led, r, g, b);
		printf("set_led %d to %d,%d,%d\n", led, r, g, b);
	    } else {
		printf("set_led: usage <led> <r> <g> <b>\n");
	    }
	} else if (strcmp(line, "show") == 0) {
	    show(neo);
	} else {
	    printf("unknown command: %s\n", line);
	}
    }
}
