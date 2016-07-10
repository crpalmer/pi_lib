#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "track.h"

int
main(int argc, char **argv)
{
    track_t **tracks = fatal_malloc(sizeof(*tracks) * argc);
    size_t i;
    audio_device_t dev;

    audio_device_init_playback(&dev);

    for (i = 1; i < argc; i++) {
	if (i + 1 < argc && strcmp(argv[i], "-c") == 0) {
	    audio_device_init(&dev, atoi(argv[i+1]), 0, true);
	    i++;
	    continue;
	}

	printf("Loading: %s\n", argv[i]);
	if ((tracks[i] = track_new(argv[i])) == NULL) {
	    perror(argv[i]);
	    exit(1);
	}
    }

    for (i = 1; i < argc; i++) {
	printf("Playing: %s\n", argv[i]);
	track_play(tracks[i]);
    }

    for (i = 1; i < argc; i++) {
	track_destroy(tracks[i]);
    }
    free(tracks);

    return 0;
}
