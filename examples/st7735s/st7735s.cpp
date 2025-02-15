#include <stdio.h>
#include <stdlib.h>
#include "gp-output.h"
#include "mem.h"
#include "pi.h"
//#include "canvas-png.h"
#include "st7735s.h"

int
main(int argc, char **argv)
{
    pi_init();
    pi_gpio_init();

    Output *reset = new GPOutput(7);
    Output *bl = new GPOutput(6);
    Output *dc = new GPOutput(8);

    spi_init_bus(1, 10, -1, 11);
    SPI *spi = new SPI(1, 9, dc);

    Display *display = new ST7735S(spi, reset, bl);
    Canvas *canvas = display->create_canvas();

#if 0
    int w = canvas->get_width();
    int h = canvas->get_height();
 
    for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
	    canvas->set_pixel(x, y, x, y, 0);
	}
    }

    canvas->flush();
#elif 0
   //canvas->nine_segment_2(29);
   canvas->left_right_line(0, 0, 90, 20, COLOR_WHITE);
   canvas->flush();
#elif 1
    for (int s = 30; s >= 0; s--) {
	ms_sleep(1000);
	canvas->blank();
	if (s >= 10) canvas->nine_segment_2(s, COLOR_WHITE);
	else canvas->nine_segment_1(s, COLOR_WHITE);
	canvas->flush();
    }
#else
    CanvasPNG *flash = new CanvasPNG("flash.png");
    if (! flash->is_valid()) {
	fprintf(stderr, "failed to load flash\n");
	exit(0);
    }
    canvas->blank();
    canvas->import(flash);
    canvas->flush();
#endif

}
