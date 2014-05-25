#include <stdio.h>
#include "wav.h"

int
main(int argc, char **argv)
{
    wav_t *w;

    w = wav_new(argv[1]);
    wav_set_volume(75);
    wav_play(w);

    return 0;
}
