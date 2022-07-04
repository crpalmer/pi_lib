#include <stdio.h>
#include <stdlib.h>
#include "externals/PIGPIO/pigpio.h"
#include "mem.h"
#include "pi.h"
#include "st7735s.h"

int
main(int argc, char **argv)
{
   int w, h;

    pi_init();
    if (gpioInitialise() < 0) {
	fprintf(stderr, "Failed to init pigpio\n");
	exit(1);
    }

    ST7735S *display = new ST7735S();
    ST7735S_Canvas *canvas = display->create_canvas();

    w = canvas->get_width();
    h = canvas->get_height();
 
    for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
	    canvas->set_pixel(x, y, x, y, 0);
	}
    }

    display->paint(canvas);
}
