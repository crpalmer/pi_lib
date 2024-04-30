#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "time-utils.h"

#include "neopixel-pi.h"

int n_leds;

inline int led(int l)
{
    return (l+n_leds) % n_leds;
}

int
main(int argc, char **argv)
{
   NeoPixelPI *neo = new NeoPixelPI();

   if (argc == 2 && strcmp(argv[1], "bootsel") == 0) {
	neo->reboot_bootsel();
	exit(0);
   }

   if (argc < 3) {
usage:
	fprintf(stderr, "usage: <n> <mode>\nMode is: chase | all <r> <g> <b> | pulse <r> <g> <b>\n");
	fprintf(stderr, "   or: bootsel\n");
        exit(1);
   }

   n_leds = atoi(argv[1]);
   if (n_leds <= 0) goto usage;

   neo->set_n_leds(n_leds);

   if (strcmp(argv[2], "chase") == 0) {
	int cur = 0;

	neo->set_all(0, 0, 0);

#define CHASE_AT_COLOUR       255, 255, 255
#define CHASE_ARRIVING_COLOUR  16,  16,  16
#define CHASE_LEAVING_COLOUR   16,  16,  16
#define CHASE_MS 20

	while (1) {
	    neo->set_led(led(cur),   CHASE_AT_COLOUR);
	    neo->set_led(led(cur+1), CHASE_ARRIVING_COLOUR);
	    neo->show();
	    ms_sleep(CHASE_MS);
	    neo->set_led(led(cur-1), CHASE_LEAVING_COLOUR);
	    neo->show();
	    ms_sleep(CHASE_MS);
	    neo->set_led(led(cur-1), 0, 0, 0);
	    cur = led(cur+1);
	}
   } else if (strcmp(argv[2], "all") == 0 && argc == 6) {
	unsigned char r, g, b;
	r = atoi(argv[3]);
	g = atoi(argv[4]);
	b = atoi(argv[5]);
	neo->set_all(r, g, b);
	neo->show();
   } else if (strcmp(argv[2], "pulse") == 0 && argc == 6) {
	unsigned char r, g, b;
	r = atoi(argv[3]);
	g = atoi(argv[4]);
	b = atoi(argv[5]);
	neo->set_all(r, g, b);
	while (1) {
	    for (int brightness = 100; brightness >= 0; brightness--) {
	        neo->set_brightness(brightness / 100.0);
	        neo->show();
		ms_sleep(30*brightness*brightness / 10000.0);
	    }
	}
   } else {
	goto usage;
   }
}
