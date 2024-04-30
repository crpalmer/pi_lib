#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tusb.h>
#include "pico/stdio_uart.h"
#include "pico/stdio_usb.h"
#include "neopixel-pico.h"
#include "pi.h"
#include "stdio-driver-reader.h"
#include "stdio-driver-writer.h"
#include "threads-console.h"
#include "time-utils.h"

static int n_leds = 0;
static NeoPixelPico *neo;

#define STRNCMP(a, b) strncmp(a, b, strlen(b))

#define FADE_INC	1
#define FADE_SLEEP_MS	10

static bool
get_rgb(Console *c, const char *usage, const char *line, int *r, int *g, int *b)
{
    if (sscanf(line, "%d %d %d", r, g, b) == 3) {
	return true;
    }
    c->printf("%s <r> <g> <b>\n", usage);
    return false;
}

static void
show(Console *c, NeoPixelPico *neo)
{
    struct timespec start;

    nano_gettime(&start);
    neo->show();
    c->printf("show took %d ms\n", nano_elapsed_ms_now(&start));
}

static void
fade_in(Console *c, NeoPixelPico *neo, int final_r, int final_g, int final_b)
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
    c->printf("faded in to %d,%d,%d\n", r, g, b);
}

static void
fade_out(Console *c, NeoPixelPico *neo, int r, int g, int b)
{
    while (r > 0 || g > 0 || b > 0) {
	if (r > 0) r -= FADE_INC;
	if (g > 0) g -= FADE_INC;
	if (b > 0) b -= FADE_INC;
	neo->set_all(r, g, b);
	neo->show();
	ms_sleep(FADE_SLEEP_MS);
    }
    c->printf("fade out complete.\n");
}

class CommandLineThread : public PiThread, public ThreadsConsole {
public:
    CommandLineThread(stdio_driver_t *driver, const char *name) : PiThread(name), ThreadsConsole(new StdioDriverReader(driver, 1024), new StdioDriverWriter(driver)) {
	start();
    }

    void main(void) override { ThreadsConsole::main(); }

    void process_cmd(const char *line) override {
	int r, g, b;

	int space = 0;
	while (line[space] && line[space] != ' ') space++;

	if (strcmp(line, "bootsel") == 0) {
	    pi_reboot_bootloader();
	} else if (strcmp(line, "dump") == 0) {
	    for (int i = 0; i < neo->get_n_leds(); i++) {
		neopixel_rgb_t rgb = neo->get_led(i);
		this->printf("%3d %02x %02x %02x\n", i, rgb.r, rgb.g, rgb.b);
	    }
	} else if (strcmp(line, "help") == 0) {
	    usage();
	} else if (STRNCMP(line, "set_n_leds ") == 0) {
	    n_leds = atoi(&line[space]);
	    neo->set_n_leds(n_leds);
	} else if (n_leds == 0) {
	    this->printf("error: you must set_n_leds <n> first\n");
	} else if (STRNCMP(line, "set_brightness ") == 0) {
	    neo->set_brightness(atof(&line[space]));
	} else if (STRNCMP(line, "fade_in ") == 0) {
	    if (get_rgb(this, "fade_in: usage", &line[space], &r, &g, &b)) {
		fade_in(this, neo, r, g, b);
	    }
	} else if (STRNCMP(line, "fade_out ") == 0) {
	    if (get_rgb(this, "fade_in: usage", &line[space], &r, &g, &b)) {
		fade_out(this, neo, r, g, b);
	    }
	} else if (STRNCMP(line, "set_all ") == 0) {
	    if (get_rgb(this, "set_all: usage", &line[space], &r, &g, &b)) {
	        neo->set_all(r, g, b);
		show(this, neo);
		this->printf("set_all to %d,%d,%d\n", r, g, b);
	    }
	} else if (STRNCMP(line, "set_led ") == 0) {
	    int led, r, g, b;

	    if (sscanf(&line[space], "%d %d %d %d", &led, &r, &g, &b) == 4) {
	        neo->set_led(led, r, g, b);
		this->printf("set_led %d to %d,%d,%d\n", led, r, g, b);
	    } else {
		this->printf("set_led: usage <led> <r> <g> <b>\n");
	    }
	} else if (strcmp(line, "show") == 0) {
	    show(this, neo);
	} else {
	    ThreadsConsole::process_cmd(line);
	}
    }

    void usage() {
	ThreadsConsole::usage();
	this->printf("bootsel: reboot into bootloader mode\n");
	this->printf("dump\n");
	this->printf("fade_in <r> <g> <b>\n");
	this->printf("fade_out <r> <g> <b>\n");
	this->printf("set_all <r> <g> <b>\n");
	this->printf("set_brightness <pct>\n");
	this->printf("set_led <led> <r> <g> <b>\n");
	this->printf("set_n_leds <n>\n");
	this->printf("show\n");
    }
};

static void threads_main(int argc, char **argv) {
    neo = new NeoPixelPico(0);
    new CommandLineThread(&stdio_uart, "uart-cmd");
    new CommandLineThread(&stdio_usb,  "usb-cmd");
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);

}
