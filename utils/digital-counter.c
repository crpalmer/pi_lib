#include <stdio.h>
#include <stdlib.h>
#include "wb.h"
#include "digital-counter.h"

static char line[1024];

int main(int argc, char **argv)
{
     digital_counter_t *dc;

     if (argc != 4 && argc != 5) {
	fprintf(stderr, "usage: bank inc dec reset\n");
	exit(1);
     }

     wb_init();
     dc = digital_counter_new(atoi(argv[1]), atoi(argv[2]), argc == 4 ? -1 : atoi(argv[4]), atoi(argv[3]));

     while (fgets(line, sizeof(line), stdin) != NULL) {
	if (line[0] == '=') {
	    digital_counter_set(dc, atoi(&line[1]));
	} else if (line[0] == '+') {
	    digital_counter_add(dc, atoi(&line[1]));
	} else if (line[0] == '-') {
	    digital_counter_add(dc, -atoi(&line[1]));
	} else if (line[0] == '0') {
	    digital_counter_reset(dc);
	} else {
	    fprintf(stderr, "= value / + inc / - dec / 0\n");
	}
    }
}

