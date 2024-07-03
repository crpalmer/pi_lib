#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "i2c.h"
#include "ssd1306.h"
#include "pi.h"

static Display *display;
static Canvas *canvas;
static char buf[128];

int
main()
{
    pi_init();

    i2c_init_bus(1, 2, 3, 400*1000);

    printf("Creating SSD1306\n");
    display = new SSD1306(1);
    printf("Success!\n");
    canvas = display->create_canvas();
    display->paint(canvas);

    while (pi_readline(buf, sizeof(buf)) != NULL) {
	if (strcmp(buf, "paint") == 0) {
            display->paint(canvas);
	} else if (strncmp(buf, "fill", 4) == 0) {
	    int v, x, y, w, h;
	    int n = sscanf(&buf[4], "%d %d %d %d %d", &v, &x, &y, &w, &h);
	    if (n == 1) canvas->fill(v, v, v);
	    else if (n == 3) canvas->fill(v, v, v, x, y);
	    else if (n == 5) canvas->fill(v, v, v, x, y, w, h);
	    else fprintf(stderr, "invalid number of arguments\n");
	} else if (strncmp(buf, "set", 3) == 0) {
	    int x, y, v;
	    if (sscanf(&buf[3], "%d %d %d", &v, &x, &y) != 3) {
		printf("set [0|1] x y\n");
	    } else {
		canvas->set_pixel(x, y, v);
	    }
	} else if (strcmp(buf, "bootsel") == 0) {
            pi_reboot_bootloader();
	} else if (buf[0] == '?') {
	    printf("paint - draw the current canvas\n");
	    printf("fill [0|1] [ x y [ w h ] ]- fill the canvas with the value\n");
	    printf("set x y [0|1] - set a pixel to the value\n");
	    printf("bootsel\n");
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
    }
}
