#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "talking-skull.h"
#include "time-utils.h"
#include "wav.h"

static int set_start = 0;
static struct timespec start, last_print;

static void
update(void *s_as_vp, double pos)
{
    struct timespec now;
    unsigned this_val = pos + 0.5;

    if (! set_start) {
	set_start = 1;
	nano_gettime(&start);
    }

    nano_gettime(&now);
    if (nano_elapsed_ms(&now, &last_print) > 20) {
	last_print = now;
	printf("%9.6f:%*c%*c %u\n", nano_elapsed_ms(&now, &start) / (1000.0), this_val+1, '*', 50 - this_val, '|', this_val);
    }
}

int
main(int argc, char **argv)
{
    wav_t *w;
    audio_meta_t meta;
    talking_skull_t *talking_skull;
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

    talking_skull = talking_skull_new(&meta, update, NULL);
    talking_skull_play(talking_skull, data, n_data);
sleep(30);

    wav_destroy(w);

    return 0;
}
