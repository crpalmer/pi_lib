#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "gp-output.h"
#include "i2c.h"
#include "il9341.h"
#include "ssd1306.h"
#include "st7735s.h"
#include "st7796s.h"
#include "pi.h"
#include "pi-gpio.h"
#include "time-utils.h"
#include "random-utils.h"

#include "../display-common.h"

static int w, h;
static uint8_t *next_gen;
static uint8_t *last_gen;

static bool was_alive(int x, int y) {
    int index = (y * w + x);
    return (last_gen[index/8] & (1 << (index%8))) != 0;
}

static void set_is_alive(int x, int y) {
    int index = (y * w + x);
    next_gen[index/8] |= (1 << (index%8));
}

static int get_n_neighbours(int x0, int y0, int w, int h)
{
   int n = 0;
   for (int x = x0 - 1; x <= x0 + 1; x++) {
	for (int y = y0 - 1; y <= y0 + 1; y++) {
	    if ((x != x0 || y != y0) && was_alive(x % w, y % h)) n++;
	}
    }
    return n;
}

static void start_next_generation() {
    uint8_t *temp = last_gen;
    last_gen = next_gen;
    next_gen = temp;
    memset(next_gen, 0, w*h/8);
}

int
main()
{
    pi_init();

    seed_random();

    Display *display = create_display(USE_ST7796S);
    Canvas *canvas = display->create_canvas(true);

    w = canvas->get_width();
    h = canvas->get_height();

    last_gen = (uint8_t *) fatal_malloc(w * h / 8 + 1);
    next_gen = (uint8_t *) fatal_malloc(w * h / 8 + 1);

    while (1) {
	int n_alive = 0;

	printf("Starting game\n");
        memset(next_gen, 0, w*h/8);

	for (int x = 0; x < w; x++) {
	    for (int y = 0; y < h; y++) {
		if (random_number_in_range(1,4) == 1) {
		    set_is_alive(x, y);
		    n_alive++;
		}
	    }
	}

	printf("Initial population of %d cells.\n", n_alive);

	do {
	    printf("Starting iteration\n");
	    start_next_generation();

	    n_alive = 0;
	    for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
		    canvas->set_pixel(x, y, COLOR_BLACK);
		    int n_neighbours = get_n_neighbours(x, y, w, h);
		    if (was_alive(x, y)) {
			if (n_neighbours == 2 || n_neighbours == 3) {
			    set_is_alive(x, y);
			    canvas->set_pixel(x, y, n_neighbours == 2 ? 0xff : 0, n_neighbours != 2 ? 0xff : 0, 0);
			    n_alive++;
			}
		    } else if (n_neighbours == 4) {
			set_is_alive(x, y);
			canvas->set_pixel(x, y, COLOR_BLUE);
			n_alive++;
		    }
		}
	    }
	    printf("%d alive\n", n_alive);
	    canvas->flush();
	    ms_sleep(100);
	} while (n_alive > 0);
    }
}
