#include <stdio.h>
#include <stdlib.h>
#include "externals/PIGPIO/pigpio.h"
#include "mem.h"
#include "pi.h"
#include "st7735s.h"

unsigned char *buffer;
int
main(int argc, char **argv)
{
   int w, h, bpp;

    pi_init();
    if (gpioInitialise() < 0) {
	fprintf(stderr, "Failed to init pigpio\n");
	exit(1);
    }

    st7735s_init();

    w = st7735s_get_width();
    h = st7735s_get_height();
    bpp = st7735s_get_bytes_per_pixel();
 
    buffer = fatal_malloc(w*h*bpp);

    for (int row = 0; row < h; row++) {
	for (int col = 0; col < w; col++) {
	    st7735s_pixel(&buffer[bpp*(row*w + col)], row, col, 0);
	}
    }

    st7735s_paint(buffer);
}
