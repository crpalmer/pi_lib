#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wb.h"
#include "digital-counter.h"
#include "pca9685.h"
#include "mcp23017.h"

static char line[1024];

int main(int argc, char **argv)
{
     Output *inc = NULL, *dec = NULL, *reset = NULL;

     if (argc != 5) {
usage:
	fprintf(stderr, "usage: device inc dec reset\n");
	fprintf(stderr, "  device: wb = (pins 0-15)\n");
	fprintf(stderr, "          pca = pca9685 (pins 0-15)\n");
	fprintf(stderr, "          mcp = mcp23017 (pins 0-15)\n");
	exit(1);
     }

     wb_init();

     if (strcmp(argv[1], "wb") == 0) {
	if (atoi(argv[2]) > 0) inc = wb_get_output(atoi(argv[2]));
	if (atoi(argv[3]) > 0) dec = wb_get_output(atoi(argv[3]));
	if (atoi(argv[4]) > 0) reset = wb_get_output(atoi(argv[4]));
     } else if (strcmp(argv[1], "pca") == 0) {
	PCA9685 *pca = new PCA9685();
	if (atoi(argv[2]) > 0) inc = pca->get_output(atoi(argv[2]));
	if (atoi(argv[3]) > 0) dec = pca->get_output(atoi(argv[3]));
	if (atoi(argv[4]) > 0) reset = pca->get_output(atoi(argv[4]));
     } else if (strcmp(argv[1], "mcp") == 0) {
	MCP23017 *mcp = new MCP23017();
	if (atoi(argv[2]) > 0) inc = mcp->get_output(atoi(argv[2]));
	if (atoi(argv[3]) > 0) dec = mcp->get_output(atoi(argv[3]));
	if (atoi(argv[4]) > 0) reset = mcp->get_output(atoi(argv[4]));
     } else {
	goto usage;
     }

     digital_counter_t *dc = new digital_counter_t(inc, dec, reset);

     while (fgets(line, sizeof(line), stdin) != NULL) {
	if (line[0] == '=') {
	    dc->set(atoi(&line[1]));
	} else if (line[0] == '+') {
	    dc->add(atoi(&line[1]));
	} else if (line[0] == '-') {
	    dc->add(-atoi(&line[1]));
	} else if (line[0] == '0') {
	    dc->set(0);
	} else if (line[0] == 'P') {
	    int pause = -1, reset_pause = -1, post_reset_pause = -1;
	    sscanf(&line[1], "%d %d %d", &pause, &reset_pause, &post_reset_pause);
	    dc->set_pause(pause, reset_pause, post_reset_pause);
	} else {
	    fprintf(stderr, "= value / + inc / - dec / 0 / p ms-to-pause ms-reset-pause ms-post-reset-pause\n");
	}
    }

    return 0;
}

