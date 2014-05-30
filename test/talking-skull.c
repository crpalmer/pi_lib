#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pi-usb.h"
#include "maestro.h"
#include "piface.h"
#include "wav.h"

#define SERVO_ID 0
#define SERVO_MIN 35
#define SERVO_MAX 90

#define EYES 2

typedef struct {
    maestro_t *m;
    piface_t  *p;
    double     gain;
} state_t;

static void
update_servo(void *s_as_vp, double pos)
{
    state_t *s = (state_t *) s_as_vp;

    pos *= s->gain;
    if (pos > 100) pos = 100;
    maestro_set_servo_pos(s->m, SERVO_ID, pos * (SERVO_MAX - SERVO_MIN) / 100 + SERVO_MIN);
    piface_set(s->p, EYES, pos > 50);
}

int
main(int argc, char **argv)
{
    wav_t *w;
    state_t s;

    pi_usb_init();

    s.gain = 1;
    s.p = piface_new();
    if ((s.m = maestro_new()) == NULL) {
	fprintf(stderr, "couldn't find a recognized device.\n");
	exit(1);
    }
    maestro_set_servo_is_inverted(s.m, SERVO_ID, 1);

    if (argc > 2 && strcmp(argv[1], "--gain") == 0) {
	s.gain = atof(argv[2]);
	argc -= 2;
	argv += 2;
    }

    if (argc > 1 && strcmp(argv[1], "--servo") == 0) {
	w = wav_new_with_servo_track(argv[2], update_servo, &s);
    } else {
	w = wav_new(argv[1]);
	wav_generate_servo_data(w, update_servo, &s);
    }

    wav_set_volume(75);
    wav_play(w);

    if (s.m) {
	maestro_set_servo_pos(s.m, SERVO_ID, 50);
	maestro_destroy(s.m);
    }

    return 0;
}
