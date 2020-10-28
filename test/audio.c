#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "audio.h"
#include "maestro.h"
#include "mem.h"
#include "pi-usb.h"
#include "talking-skull.h"

static double gain = 10;
static maestro_t *maestro;

#define SERVO_ID 0
#define SERVO_MIN 35
#define SERVO_MAX 90

static void
servo_update(void *unused, double pos)
{
    pos *= gain;
    if (pos > 100) pos = 100;
    if (maestro) {
	maestro_set_servo_pos(maestro, SERVO_ID, pos * (SERVO_MAX - SERVO_MIN) / 100 + SERVO_MIN);
	//piface_set(s->p, EYES, pos > 50);
    } else {
	printf("servo: %.0f\n", pos);
    }
}

int
main(int argc, char **argv)
{
    unsigned char *buffer;
    size_t size;
    audio_t *in, *out;
    audio_config_t cfg;
    audio_meta_t meta;
    audio_device_t in_dev, out_dev;
    talking_skull_t *skull;

    pi_usb_init();
    if ((maestro = maestro_new()) == NULL) {
        fprintf(stderr, "couldn't find a recognized device, disabling skull.\n");
    } else {
        maestro_set_servo_is_inverted(maestro, 0, true);
    }


    audio_config_init_default(&cfg);

    audio_device_init_playback(&in_dev);
    out = audio_new(&cfg, &in_dev);
    audio_print_controls(out, stdout);

    audio_device_init_capture(&out_dev);
    in = audio_new(&cfg, &out_dev);
    audio_print_controls(in, stdout);

    if (! out) {
	perror("out");
	fprintf(stderr, "Failed to initialize playback\n");
	exit(1);
    }

    if (! in) {
	perror("in");
	fprintf(stderr, "Failed to initialize capture\n");
	exit(1);
    }

    audio_set_volume(in, 50);
    audio_set_volume(out, 100);

    size = audio_get_buffer_size(in);
    assert(audio_get_buffer_size(out) == size);

    buffer = fatal_malloc(size);

    audio_meta_init_from_config(&meta, &cfg);
    skull = talking_skull_new(&meta, servo_update, NULL);

    fprintf(stderr, "Copying from capture to play using %u byte buffers\n", size);

    while (audio_capture_buffer(in, buffer)) {
	talking_skull_play(skull, buffer, size);
	if (! audio_play_buffer(out, buffer, size)) {
	    fprintf(stderr, "Failed to play buffer!\n");
	    exit(1);
	}
    }

    fprintf(stderr, "Failed to capture buffer!\n");

    audio_destroy(in);
    audio_destroy(out);
    free(buffer);

    return 0;
}

