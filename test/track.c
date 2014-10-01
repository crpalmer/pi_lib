#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "track.h"

int
main(int argc, char **argv)
{
    track_t **tracks = fatal_malloc(sizeof(*tracks) * argc);
    size_t i;

    for (i = 1; i < argc; i++) {
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
