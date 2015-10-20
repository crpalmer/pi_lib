#include <stdio.h>
#include <stdlib.h>
#include "maestro.h"
#include "util.h"
#include "pi-usb.h"
#include "call-every.h"

static char buf[1024];

int
main(int argc, char **argv)
{
    maestro_t *m;

    pi_usb_init();
    if ((m = maestro_new()) == NULL) {
	fprintf(stderr, "couldn't find a recognized device.\n");
	exit(1);
    }

    while (fgets(buf, sizeof(buf), stdin) != NULL && ! feof(stdin)) {
	int which, where;

	if (sscanf(buf, "%d %d", &which, &where) == 2) {
	    maestro_set_servo_pos(m, which, where);
	}
    }

    return 0;
}
