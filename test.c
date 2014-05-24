#include <stdio.h>
#include <stdlib.h>
#include "maestro.h"
#include "util.h"
#include "pi-usb.h"

#define STEP_SIZE 0.2

static void
back_and_forth(maestro_t *m, servo_id_t id)
{
    double pos;

    maestro_set_servo_is_inverted(m, id+1, 1);

    while (1) {
    	for (pos = 0; pos < 100; pos += STEP_SIZE) {
	    if (! maestro_set_servo_pos(m, id, pos)) printf("set_target failed.\n");
	    if (! maestro_set_servo_pos(m, id+1, 100-pos)) printf("set_target failed.\n");
	    ms_sleep(100 * STEP_SIZE);
	}
    	for (pos = 100; pos >= STEP_SIZE; pos -= STEP_SIZE) {
	    if (! maestro_set_servo_pos(m, id, pos)) printf("set_target failed.\n");
	    if (! maestro_set_servo_pos(m, id+1, 100-pos)) printf("set_target failed.\n");
	    ms_sleep(100 * STEP_SIZE);
	}
    }
}

int
main(int argc, char **argv)
{
    maestro_t *m;

    pi_usb_init();
    if ((m = maestro_new()) == NULL) {
	fprintf(stderr, "couldn't find a recognized device.\n");
	exit(1);
    }
   
    printf("n-servos = %d\n", maestro_n_servos(m));

    back_and_forth(m, 0);

    return 0;
}
