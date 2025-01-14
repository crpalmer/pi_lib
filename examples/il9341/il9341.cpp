#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "pi.h"
#include "gp-output.h"
#include "il9341.h"
#include "spi.h"

static Display *display;
static Canvas *canvas;
static char buf[128];

int
main()
{
    pi_init();

ms_sleep(2000);

    spi_init_bus(1, 10, -1, 11);

    printf("Creating IL9341\n");

    Output *bl = new GPOutput(6);
    Output *reset = new GPOutput(7);
    Output *dc = new GPOutput(8);

    SPI *spi = new SPI(1, 9, dc);
    display = new IL9341(spi, reset, bl);

    printf("Success!\n");
    canvas = display->create_canvas();
    display->paint(canvas);

    while (pi_readline(buf, sizeof(buf)) != NULL) {
	double pct;

	if (strcmp(buf, "paint") == 0) {
            display->paint(canvas);
 	} else if (strncmp(buf, "brightness", 10) == 0 && sscanf(&buf[10], "%lf", &pct) == 1) {
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
		if (d >= 10) canvas->nine_segment_2(d, RGB24_of(r, g, b));
		else canvas->nine_segment_1(d, RGB24_of(r, g, b));
	    } else {
		printf("9seg r g b digits\n");
	    }
	} else if (strcmp(buf, "bootsel") == 0) {
            pi_reboot_bootloader();
	} else if (buf[0] == '?') {
	    printf("paint - draw the current canvas\n");
	    printf("9seg r g b digits\n");
	    printf("brightness 0.0-1.0\n");
	    printf("fill r g b [ x y [ w h ] ]- fill the canvas with the value\n");
	    printf("set x y r g b - set a pixel to the value\n");
	    printf("bootsel\n");
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
    }
}
