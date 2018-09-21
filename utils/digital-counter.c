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
	} else if (line[0] == 'P') {
	    int pause = -1, reset_pause = -1, post_reset_pause = -1;
	    sscanf(&line[1], "%d %d %d", &pause, &reset_pause, &post_reset_pause);
	    digital_counter_set_pause(dc, pause, reset_pause, post_reset_pause);
	} else {
	    fprintf(stderr, "= value / + inc / - dec / 0 / p ms-to-pause ms-reset-pause ms-post-reset-pause\n");
	}
    }
}

