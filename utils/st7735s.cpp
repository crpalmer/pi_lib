#include <stdio.h>
#include <stdlib.h>
#include "pi-gpio.h"
#include "mem.h"
#include "pi.h"
//#include "canvas_png.h"
#include "st7735s.h"
#include "util.h"

int
main(int argc, char **argv)
{
    pi_init();
    pi_gpio_init();

    Display *display = new ST7735S();
    Canvas *canvas = display->create_canvas();

#if 0
    int w = canvas->get_width();
    int h = canvas->get_height();
 
    for (int x = 0; x < w; x++) {
	for (int y = 0; y < h; y++) {
	    canvas->set_pixel(x, y, x, y, 0);
	}
    }

    display->paint(canvas);
#elif 0
   //canvas->nine_segment_2(29);
   canvas->left_right_line(0, 0, 90, 20, WHITE);
   display->paint(canvas);
#elif 1
    for (int s = 30; s >= 0; s--) {
	ms_sleep(1000);
	canvas->blank();
	if (s >= 10) canvas->nine_segment_2(s);
	else canvas->nine_segment_1(s, 0xff0000);
	display->paint(canvas);
    }
#else
    CanvasPNG *flash = new CanvasPNG("flash.png");
    if (! flash->is_valid()) {
	fprintf(stderr, "failed to load flash\n");
	exit(0);
    }
    canvas->blank();
    canvas->import(flash);
    display->paint(canvas);
#endif

}
