#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include "i2c.h"
#include "ssd1306.h"
#include "pi.h"
#include "pi-gpio.h"
#include "time-utils.h"
#include "random-utils.h"

static Display *display;
static Canvas *canvas[2];

static int get_n_neighbours(Canvas *c, int x0, int y0, int w, int h)
{
   int n = 0;
   for (int x = x0 - 1; x <= x0 + 1; x++) {
	for (int y = y0 - 1; y <= y0 + 1; y++) {
	    if ((x != x0 || y != y0) && c->get_pixel(x % w, y % h)) n++;
	}
    }
    return n;
}

int
main()
{
    pi_init();

    seed_random();

    i2c_init_bus(1, 2, 3, 400*1000);

    printf("Creating SSD1306\n");
    display = new SSD1306(1);
    printf("Success!\n");
    canvas[0] = display->create_canvas();
    canvas[1] = display->create_canvas();

    int w = canvas[0]->get_width();
    int h = canvas[0]->get_height();

    while (1) {
        int index = 0;
	int n_alive = 0;

	printf("Starting game\n");

	for (int x = 0; x < w; x++) {
	    for (int y = 0; y < h; y++) {
		if (random_number_in_range(1,4) == 1) {
		    canvas[index]->set_pixel24(x, y, WHITE);
		    n_alive++;
		}
	    }
	}

	printf("Initial population of %d cells.\n", n_alive);

	do {
	    Canvas *c_last = canvas[index];
	    index = (index + 1) % 2;
	    Canvas *c_next = canvas[index];
	    printf("Starting iteration with index = %d\n", index);

	    c_next->fill24(BLACK);
	    n_alive = 0;
	    for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
		    int n_neighbours = get_n_neighbours(c_last, x, y, w, h);
		    if (c_last->get_pixel(x, y)) {
			if (n_neighbours == 2 || n_neighbours == 3) {
			    c_next->set_pixel24(x, y, WHITE);
			    n_alive++;
			}
		    } else if (n_neighbours == 3) {
			c_next->set_pixel24(x, y, WHITE);
			n_alive++;
		    }
		}
	    }
	    printf("%d alive\n", n_alive);
	    display->paint(c_next);
	    ms_sleep(100);
	} while (n_alive > 0);
    }
}
