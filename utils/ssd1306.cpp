#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "i2c.h"
#include "ssd1306.h"
#include "pi.h"
#include "util.h"

static Display *display;
static Canvas *canvas;
static char buf[128];

#ifdef PI_PICO

#include <pico/bootrom.h>

char *readline(char *buf, size_t n)
{
    pico_readline_echo(buf, n, true);
    printf("\n");
    return buf;
}

#else

#include <ctype.h>

char *readline(char *buf, size_t n)
{
    if (feof(stdin)) return NULL;
    if (fgets(buf, n, stdin) == NULL) return NULL;
    int i;
    for (i = strlen(buf); i > 0 && isspace(buf[i-1]); i--) {}
    buf[i] = '\0';
    return buf;
}

#endif

int
main()
{
    pi_init();

    i2c_init_bus(1, 400*1000);
    i2c_config_gpios(2, 3);

    printf("Creating SSD1306\n");
    display = new SSD1306(1);
    printf("Success!\n");
    canvas = display->create_canvas();
    display->paint(canvas);

    while (readline(buf, sizeof(buf)) != NULL) {
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
#ifdef PI_PICO
	} else if (strcmp(buf, "bootsel") == 0) {
            printf("Rebooting into bootloader mode...\n");
            reset_usb_boot(1<<PICO_DEFAULT_LED_PIN, 0);
#endif
	} else if (buf[0] == '?') {
	    printf("paint - draw the current canvas\n");
	    printf("fill [0|1] [ x y [ w h ] ]- fill the canvas with the value\n");
	    printf("set x y [0|1] - set a pixel to the value\n");
#ifdef PI_PICO
	    printf("bootsel\n");
#endif
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
    }
}
