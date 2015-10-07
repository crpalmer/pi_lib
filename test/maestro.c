#include <stdio.h>
#include <stdlib.h>
#include "maestro.h"
#include "util.h"
#include "pi-usb.h"
#include "call-every.h"

#define STEP_MS   100
#define STEP_SIZE (100.0 / 5 / 1000 * STEP_MS)

typedef struct {
    maestro_t *m;
    double pos;
    double step;
} servo_state_t;

static void
update_servos(void *s_as_vp)
{
    servo_state_t *s = (servo_state_t *) s_as_vp;

    s->pos += s->step;
    if (s->pos < 0) {
	s->pos = 0;
	s->step = -s->step;
    } else if (s->pos >= 100) {
	s->pos = 100-STEP_SIZE;
	s->step= -s->step;
    }

    printf("pos = %f\n", s->pos);

    if (! maestro_set_servo_pos(s->m, 0, s->pos)) printf("set_target failed.\n");
    if (! maestro_set_servo_pos(s->m, 1, s->pos)) printf("set_target failed.\n");
}

int
main(int argc, char **argv)
{
    maestro_t *m;
    call_every_t *e;
    servo_state_t s;

    pi_usb_init();
    if ((m = maestro_new()) == NULL) {
	fprintf(stderr, "couldn't find a recognized device.\n");
	exit(1);
    }

    //maestro_set_range(m, 0, TALKING_SKULL);
   
    printf("n-servos = %d\n", maestro_n_servos(m));

    e = call_every_new(STEP_MS, update_servos, (void *) &s);

    maestro_set_servo_is_inverted(m, 1, 1);

    s.m = m;
    s.pos = 50;
    s.step = STEP_SIZE;

    call_every_start(e);
    while (1) sleep(1000);

    return 0;
}
