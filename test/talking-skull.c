#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "pi-usb.h"
#include "maestro.h"
#include "piface.h"
#include "talking-skull.h"
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
    audio_meta_t meta;
    state_t s;
    audio_config_t audio_cfg;
    audio_device_t audio_dev;
    audio_t *audio;
    talking_skull_t *talking_skull;
    bool has_servo_track;

    pi_usb_init();

    s.gain = 1;
    s.p = piface_new();
    if ((s.m = maestro_new()) == NULL) {
	fprintf(stderr, "couldn't find a recognized device.\n");
	exit(1);
    }
    maestro_set_servo_is_inverted(s.m, SERVO_ID, 1);

    while (argc > 1 && argv[1][0] == '-' && argv[1][1] == '-') {
	if (strcmp(argv[1], "--") == 0) {
	    argc -= 1;
	    argv += 1;
	    break;
	}
	if (argc > 2 && strcmp(argv[1], "--gain") == 0) {
	    s.gain = atof(argv[2]);
	    argc -= 2;
	    argv += 2;
	}

	if (argc > 1 && strcmp(argv[1], "--servo") == 0) {
	    has_servo_track = true;
	    argc -= 1;
	    argv += 1;
	}
    }

    w = wav_new(argv[1]);
    if (! w) {
	perror(argv[1]);
	exit(1);
    }

    meta = wav_get_meta(w);
    talking_skull = talking_skull_new(&meta, has_servo_track, update_servo, &s);
    if (has_servo_track) {
	wav_extract_servo_track(w);
    }

    audio_device_init_playback(&audio_dev);
    audio_config_init_default(&audio_cfg);
    wav_configure_audio(w, &audio_cfg);

    printf("Config from wav file: ");
    audio_config_print(&audio_cfg, stdout);
    printf("\n");

    audio = audio_new(&audio_cfg, &audio_dev);

    if (! audio) {
	perror("audio_new");
	exit(1);
    }

    audio_set_volume(audio, 100);

    wav_play_with_talking_skull(w, audio, talking_skull);

    if (s.m) {
	maestro_set_servo_pos(s.m, SERVO_ID, 50);
	maestro_destroy(s.m);
    }

    wav_destroy(w);
    audio_destroy(audio);
    return 0;
}
