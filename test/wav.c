#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pi-usb.h"
#include "maestro.h"
#include "wav.h"

#define SERVO_ID 0
#define SERVO_MIN 35
#define SERVO_MAX 90

static void
update_servo(void *m_as_vp, double pos)
{
    maestro_t *m = (maestro_t *) m_as_vp;

    maestro_set_servo_pos(m, SERVO_ID, pos * (SERVO_MAX - SERVO_MIN) / 100 + SERVO_MIN);
}

int
main(int argc, char **argv)
{
    wav_t *w;
    maestro_t *m = NULL;

    if (argc > 1 && strcmp(argv[1], "--servo") == 0) {
	pi_usb_init();
	if ((m = maestro_new()) == NULL) {
	    fprintf(stderr, "couldn't find a recognized device.\n");
	    exit(1);
	}
	maestro_set_servo_is_inverted(m, SERVO_ID, 1);
	w = wav_new_with_servo_track(argv[2], update_servo, m);
    } else {
	w = wav_new(argv[1]);
    }

    wav_set_volume(75);
    wav_play(w);

    maestro_set_servo_pos(m, SERVO_ID, 50);

    if (m) maestro_destroy(m);

    return 0;
}
