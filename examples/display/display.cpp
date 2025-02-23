#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "pi.h"
#include "gp-output.h"
#include "i2c.h"
#include "il9341.h"
#include "image-png.h"
#include "pi-threads.h"
#include "ssd1306.h"
#include "st7735s.h"
#include "st7796s.h"
#include "spi.h"

#include "../display-common.h"

static char buf[128];

static void threads_main(int argc, char **argv) {
    Display *display = create_display(USE_ST7735S);
    //Display *display = create_display(USE_ST7796S);
    //Display *display = create_display(USE_IL9341);
    Canvas *canvas = display->create_canvas(true);

    while (pi_readline(buf, sizeof(buf)) != NULL) {
	double pct;

 	if (strncmp(buf, "brightness", 10) == 0 && sscanf(&buf[10], "%lf", &pct) == 1) {
	    display->set_brightness(pct);
	} else if (strncmp(buf, "fill", 4) == 0) {
	    int r, g, b, x, y, w, h;
	    int n = sscanf(&buf[4], "%d %d %d %d %d %d %d", &r, &g, &b, &x, &y, &w, &h);
	    if (n == 3) canvas->fill(r, g, b);
	    else if (n == 5) canvas->fill(r, g, b, x, y);
	    else if (n == 7) canvas->fill(r, g, b, x, y, w, h);
	    else fprintf(stderr, "invalid number of arguments\n");
	} else if (strncmp(buf, "set", 3) == 0) {
	    int x, y, r, g, b;
	    if (sscanf(&buf[3], "%d %d %d %d %d", &r, &g, &b, &x, &y) != 6) {
		printf("set r g b x y\n");
	    } else {
		canvas->set_pixel(x, y, r, g, b);
	    }
	} else if (strncmp(buf, "9seg", 4) == 0) {
	    int r, g, b, d;
	    if (sscanf(&buf[4], "%d %d %d %d", &r, &g, &b, &d) == 4) {
		if (d >= 10) canvas->nine_segment_2(d, r, g, b);
		else canvas->nine_segment_1(d, r, g, b);
	    } else {
		printf("9seg r g b digits\n");
	    }
	} else if (strncmp(buf, "png ", 4) == 0) {
	    Image *png = image_png_load(&buf[4]);
	    if (png) {
		canvas->import(png);
	    	canvas->flush();
		delete png;
	    }
	} else if (strcmp(buf, "free") == 0) {
            printf("%d free bytes\n", pi_threads_get_free_ram());
	} else if (strcmp(buf, "bootsel") == 0) {
            pi_reboot_bootloader();
	} else if (buf[0] == '?') {
	    printf("9seg r g b digits\n");
	    printf("brightness 0.0-1.0\n");
	    printf("fill r g b [ x y [ w h ] ]- fill the canvas with the value\n");
	    printf("set x y r g b - set a pixel to the value\n");
	    printf("png filename\n");
	    printf("free\n");
	    printf("bootsel\n");
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
	canvas->flush();
    }
}

int
main()
{
    pi_init_with_threads(threads_main, 0, NULL);
}
