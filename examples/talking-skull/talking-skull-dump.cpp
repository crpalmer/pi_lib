#include <stdio.h>
#include "pi.h"
#include "talking-skull-from-audio.h"
#include "talking-skull-from-file.h"
#include "wav.h"

int
main(int argc, char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "usage: filename.wav\n");
	return 1;
    }

    TalkingSkullOps *ops = TalkingSkullAudioOps::open_wav(argv[1]);

    double s_per_i = ops->get_usec_per_i() / (1000.0*1000);
    double pos;
    double t = 0;

    while (ops->next(&pos)) {
	t += s_per_i;
	printf("%.3lf %6.2lf\n", t, pos);
    }

    return 0;
}
