#include <stdio.h>
#include "talking-skull-from-audio.h"
#include "wav.h"

int
main(int argc, char **argv)
{
    Wav *wav;
    TalkingSkullAudioOps *ops;

    if (argc != 2) {
	fprintf(stderr, "usage: filename.wav\n");
	return 1;
    }

    wav = new Wav(new BufferFile(argv[1]));
    ops = new TalkingSkullAudioOps(wav->to_audio_buffer());

    talking_skull_ops_to_file(stdout, ops);

    return 0;
}
