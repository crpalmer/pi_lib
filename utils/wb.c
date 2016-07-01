#include <stdio.h>
#include <stdlib.h>
#include "wb.h"

static char buf[1024];

int
main(int argc, char **argv)
{
    if (wb_init() < 0) {
	fprintf(stderr, "Failed to initialize wb\n");
	exit(1);
    }

    while (fgets(buf, sizeof(buf), stdin) != NULL && ! feof(stdin)) {
	int bank, pin, value;

	if (buf[0] == 'g') {
	    if (sscanf(&buf[1], "%d", &pin) == 1) {
		printf("%d = %d\n", pin, wb_get(pin));
	    } else {
		value = wb_get_all();
		for (pin = 7; pin >= 0; pin--) {
		    printf("%d", (value & (1<<pin)) != 0);
		}
		printf("\n");
	    }
	} else if (buf[0] == 's') {
	    if (sscanf(&buf[1], "%d %d %d", &bank, &pin, &value) == 3) {
		wb_set(WB_OUTPUT(bank, pin), value);
	    } else {
		goto usage;
	    }
	} else if (buf[0] == 'p') {
	    unsigned freq;
	    float fvalue;
	    if (sscanf(&buf[1], "%d %d %u %f", &bank, &pin, &freq, &fvalue) == 4) {
		wb_pwm_freq(WB_OUTPUT(bank, pin), freq, fvalue/100.0);
	    } else if (sscanf(&buf[1], "%d %d %f", &bank, &pin, &fvalue) == 3) {
		wb_pwm(WB_OUTPUT(bank, pin), fvalue/100.0);
	    } else {
		goto usage;
	    }
	} else if (buf[0] == 'v') {
	    if (sscanf(&buf[1], "%d %d %d", &bank, &pin, &value) == 3) {
		wb_servo(WB_OUTPUT(bank, pin), value);
	    } else {
		goto usage;
	    }
	} else {
usage:
	    fprintf(stderr, "g [<pin 1-8>]\np <bank 1/2> <pin 1-8> [<freq>] <duty%%>\ns <bank 1/2> <pin 1-8> <value 0/1>\nv <bank 1/2> <pin 1-8> <servo pulse width in us>\n");
	}
    }

    return 0;
}
