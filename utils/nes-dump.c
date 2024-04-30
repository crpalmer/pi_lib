#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include "pi.h"
#include "nes.h"

#define F_event "/dev/input/by-id/usb-0079_Controller-event-joystick"
#define F_legacy "/dev/input/by-id/usb-0079_Controller-joystick"

const char *dir_to_string(int dir)
{
    return dir > 0 ? "pushed" : "released";
}

int
main(int argc, char **argv)
{
    FILE *f;
    nes_event_t e;
    int (*do_read)(nes_event_t *, FILE *) = nes_read;
    const char *fname = F_event;

    if (argc > 1 && strcmp(argv[1], "--legacy") == 0) {
	do_read = nes_read_legacy;
	fname = F_legacy;
    }

    printf("Using: %s\n", fname);
    if ((f = file_open_read(fname)) == 0) {
	perror(fname);
	exit(0);
    }

    for (;;) {
	int status = do_read(&e, f);

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
    file_close(f);
}
