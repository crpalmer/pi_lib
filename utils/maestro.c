#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "maestro.h"
#include "pi-usb.h"

static char buf[1024];
static char str[1024];

const struct {
    const char *str;
    maestro_range_t value;
} ranges[] = {
    { "standard", STANDARD_SERVO },
    { "extended", EXTENDED_SERVO },
    { "talking-skull", TALKING_SKULL },
    { "talking-deer", TALKING_DEER },
    { "talking-skull2", TALKING_SKULL2 },
    { "baxter-mouth", BAXTER_MOUTH },
    { "baxter-head", BAXTER_HEAD },
    { "baxter-tail", BAXTER_TAIL },
    { "hs65", HITEC_HS65 },
    { "hs81", HITEC_HS81 },
    { "hs425", HITEC_HS425 },
    { "parallax", PARALLAX_STANDARD },
    { "ds3218", SERVO_DS3218 },
    { NULL, -1 }
};

int
main(int argc, char **argv)
{
    maestro_t *m;
    int i;

    pi_usb_init();

    if ((m = maestro_new()) == NULL) {
	fprintf(stderr, "Failed to initialize servo controller\n");
	exit(1);
    }

    while (fgets(buf, sizeof(buf), stdin) != NULL && ! feof(stdin)) {
	int c, d, dd;

	if (buf[0] == 'i') {
	    d = 1;
	    if (sscanf(&buf[1], "%d %d", &c, &d) >= 1) {
		maestro_set_servo_is_inverted(m, c, d);
		continue;
	    }
	} if (buf[0] == 'r') {
	    if (sscanf(&buf[1], "%d %d %d", &c, &d, &dd) == 3) {
		maestro_set_servo_range_pct(m, c, d, dd);
		continue;
	    }
	} if (buf[0] == 'R') {
	    if (sscanf(&buf[1], "%d %d %d", &c, &d, &dd) == 3) {
		maestro_set_servo_physical_range(m, c, d, dd);
		continue;
	    } else if (sscanf(&buf[1], "%d %s", &c, str) == 2) {
		for (i = 0; ranges[i].str && strcasecmp(ranges[i].str, str) != 0; i++) {}
		if (ranges[i].str) {
		    maestro_set_servo_range(m, c, ranges[i].value);
		    continue;
		}
	    }
	} else if (buf[0] == 's') {
	    if (sscanf(&buf[1], "%d %d", &c, &d) == 2) {
		maestro_set_servo_speed(m, c, d);
		continue;
	    }
	} else if (strcmp(buf, "factory reset\n") == 0) {
	    printf("Resetting the config and restarting the controller\n");
	    maestro_factory_reset(m);
	    continue;
	} else if (sscanf(buf, "%d %d", &c, &d) == 2) {
	    maestro_set_servo_pos(m, c, d);
	    continue;
	}

        fprintf(stderr, "i channel [0/1] - set is_inverted\n");
        fprintf(stderr, "r channel low%% high%% - set range\n");
	fprintf(stderr, "R channel min_us max_us - set physical range\n");
	fprintf(stderr, "R channel name - range by name:");
	for (i = 0; ranges[i].str; i++) fprintf(stderr, " %s", ranges[i].str);
	fprintf(stderr, "\n");
        fprintf(stderr, "s channel ms - set speed (ms for whole range)\n");
	fprintf(stderr, "factory reset\n");
	fprintf(stderr, "channel pos%%\n");
    }

    return 0;
}
