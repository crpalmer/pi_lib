#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pico/bootrom.h>
#include <tusb.h>
#include "neopixel-pico.h"
#include "pi.h"
#include "time-utils.h"
#include "util.h"

static char line[100*1024];

#define STRNCMP(a, b) strncmp(a, b, strlen(b))

int
main()
{
    bool echo = false;

    pi_init_no_reboot();

    NeoPixelPico *neo = new NeoPixelPico(0);

    for (;;) {
	while (!tud_cdc_connected()) {
	    ms_sleep(1);
	}

	pico_readline_echo(line, sizeof(line), echo);

	int space = 0;
	while (line[space] && line[space] != ' ') space++;

	if (strcmp(line, "bootsel") == 0) {
	    printf("Rebooting into bootloader mode...\n");
	    reset_usb_boot(1<<PICO_DEFAULT_LED_PIN, 0);
	} else if (strcmp(line, "dump") == 0) {
	    for (int i = 0; i < neo->get_n_leds(); i++) {
		neopixel_rgb_t rgb = neo->get_led(i);
		printf("%3d %02x %02x %02x\n", i, rgb.r, rgb.g, rgb.b);
	    }
	} else if (STRNCMP(line, "echo ") == 0) {
	    echo = atoi(&line[space]);
	} else if (STRNCMP(line, "set_brightness ") == 0) {
	    neo->set_brightness(atof(&line[space]));
	} else if (STRNCMP(line, "set_led ") == 0) {
	    int led, r, g, b;

	    if (sscanf(&line[space], "%d %d %d %d", &led, &r, &g, &b) == 4) {
	        neo->set_led(led, r, g, b);
		if (echo) printf("set_led %d to %d,%d,%d\n", led, r, g, b);
	    } else if (echo) {
		printf("set_led: usage <led> <r> <g> <b>\n");
	    }
	} else if (STRNCMP(line, "set_n_leds ") == 0) {
	    neo->set_n_leds(atoi(&line[space]));
	} else if (strcmp(line, "show") == 0) {
	    struct timespec start;

	    if (echo) nano_gettime(&start);
	    neo->show();
	    if (echo) printf("show took %d ms\n", nano_elapsed_ms_now(&start));
	} else if (strcmp(line, "help") == 0) {
	    printf("bootsel: reboot into bootloader mode\n");
	    printf("dump\n");
	    printf("echo <0/1>: set echo mode\n");
	    printf("set_brightness <pct>\n");
	    printf("set_led <led> <r> <g> <b>\n");
	    printf("set_n_leds <n>\n");
	    printf("show\n");
	} else {
	    printf("unknown command: %s\n", line);
	}
    }
}
