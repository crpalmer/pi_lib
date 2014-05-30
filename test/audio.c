#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mem.h"
#include "audio.h"

int
main(int argc, char **argv)
{
    unsigned char *buffer;
    size_t size;
    audio_t *in, *out;
    audio_config_t cfg;
    audio_device_t in_dev, out_dev;

    audio_config_init_default(&cfg);
    audio_device_init_playback(&in_dev);
    audio_device_init_capture(&out_dev);

    out = audio_new(&cfg, &in_dev);
    in = audio_new(&cfg, &out_dev);

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

    fprintf(stderr, "Copying from capture to play using %u byte buffers\n", size);

    while (audio_capture_buffer(in, buffer)) {
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

