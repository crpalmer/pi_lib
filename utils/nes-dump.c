#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include "nes.h"

#define F "/dev/input/by-id/usb-0079_Controller-event-joystick"

const char *dir_to_string(int dir)
{
    return dir > 0 ? "pushed" : "released";
}

int
main(int argc, char **argv)
{
    FILE *f;
    nes_event_t e;

    if ((f = fopen(F, "r")) == 0) {
	perror(F);
	exit(0);
    }

    for (;;) {
	int status = nes_read(&e, f);

	if (status > 0) {
	    switch(e.button) {
	    case NES_A: printf("A %s\n", dir_to_string(e.dir)); break;
	    case NES_B: printf("B %s\n", dir_to_string(e.dir)); break;
	    case NES_START: printf("START %s\n", dir_to_string(e.dir)); break;
	    case NES_SELECT: printf("SELECT %s\n", dir_to_string(e.dir)); break;
	    case NES_LEFT_RIGHT: printf("L/R %s\n", e.dir == 0 ? "released" : (e.dir > 0 ? "right" : "left")); break;
	    case NES_UP_DOWN: printf("U/D %s\n", e.dir == 0 ? "released" : (e.dir > 0 ? "down" : "up")); break;
	    }
        } else if (status < 0) {
	    break;
	}
    }
    fclose(f);
}
