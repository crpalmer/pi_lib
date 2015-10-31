#include <stdio.h>
#include <stdlib.h>
#include "wb.h"

static char buf[1024];

int
main(int argc, char **argv)
{
    wb_t *wb;

    wb = wb_new();

    while (fgets(buf, sizeof(buf), stdin) != NULL && ! feof(stdin)) {
	int bank, pin, value;

	if (buf[0] == 'g') {
	    if (sscanf(&buf[1], "%d", &pin) == 1) {
		printf("%d = %d\n", pin, wb_get(wb, pin));
	    } else {
		goto usage;
	    }
	} else if (buf[0] == 's') {
	    if (sscanf(&buf[1], "%d %d %d", &bank, &pin, &value) == 3) {
		wb_set(wb, WB_OUTPUT(bank, pin), value);
	    } else {
		goto usage;
	    }
	} else {
usage:
	    fprintf(stderr, "g <pin>\ns <bank> <pin> <value>\n");
	}
    }

    wb_destroy(wb);

    return 0;
}
