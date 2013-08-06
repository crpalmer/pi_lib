#include <stdio.h>
#include <stdlib.h>
#include "maestro.h"
#include "util.h"
#include "pi-usb.h"

static void
back_and_forth(maestro_t *m, servo_id_t id)
{
    maestro_set_servo_is_inverted(m, id+1, 1);

    while (1) {
    	for (unsigned char pos = 0; pos < 100; pos += 10) {
	    if (! maestro_set_servo_pos(m, id, pos)) printf("set_target failed.\n");
	    if (! maestro_set_servo_pos(m, id+1, 100-pos)) printf("set_target failed.\n");
	    ms_sleep(100);
	}
    	for (unsigned char pos = 100; pos >= 10; pos -= 10) {
	    if (! maestro_set_servo_pos(m, id, pos)) printf("set_target failed.\n");
	    if (! maestro_set_servo_pos(m, id+1, 100-pos)) printf("set_target failed.\n");
	    ms_sleep(100);
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
