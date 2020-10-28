#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "talking-skull.h"
#include "time-utils.h"
#include "wav.h"

int
main(int argc, char **argv)
{
    wav_t *w;
    audio_meta_t meta;
    talking_skull_t *talking_skull;
    servo_operations_t *ops;
    void *data;
    size_t n_data;

    if (argc != 2) {
	fprintf(stderr, "usage: filename.wav\n");
	return 1;
    }

    w = wav_new(argv[1]);
    if (! w) {
	perror(argv[1]);
	return 1;
    }

    meta = wav_get_meta(w);
    data = wav_get_raw_data(w, &n_data);

    talking_skull = talking_skull_new(&meta, NULL, NULL);
    ops = talking_skull_prepare(talking_skull, data, n_data);

    servo_operations_save_f(ops, stdout);

    wav_destroy(w);

    return 0;
}
