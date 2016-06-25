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
		printf("%d = %d\n", pin, wb_get(pin-1));
	    } else {
		value = wb_get_all();
		for (pin = 7; pin >= 0; pin--) {
		    printf("%d", (value & (1<<pin)) != 0);
		}
		printf("\n");
	    }
	} else if (buf[0] == 's') {
	    if (sscanf(&buf[1], "%d %d %d", &bank, &pin, &value) == 3) {
		wb_set(WB_OUTPUT(bank-1, pin-1), value);
	    } else {
		goto usage;
	    }
	} else {
usage:
	    fprintf(stderr, "g [<pin 1-8>]\ns <bank 1/2> <pin 1-8> <value 0/1>\n");
	}
    }

    return 0;
}
