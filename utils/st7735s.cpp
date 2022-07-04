#include <stdio.h>
#include <stdlib.h>
#include "externals/PIGPIO/pigpio.h"
#include "mem.h"
#include "pi.h"
#include "st7735s.h"
#include "util.h"

int
main(int argc, char **argv)
{
    pi_init();
    if (gpioInitialise() < 0) {
	fprintf(stderr, "Failed to init pigpio\n");
	exit(1);
    }

    ST7735S *display = new ST7735S();
    ST7735S_Canvas *canvas = display->create_canvas();

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
#else
    for (int s = 30; s >= 0; s--) {
	ms_sleep(1000);
	canvas->blank();
	if (s >= 10) canvas->nine_segment_2(s);
	else canvas->nine_segment_1(s, 0xff0000);
	display->paint(canvas);
    }
#endif

}
