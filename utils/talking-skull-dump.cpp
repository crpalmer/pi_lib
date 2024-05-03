#include <stdio.h>
#include "pi.h"
#include "talking-skull-from-audio.h"
#include "wav.h"

int
main(int argc, char **argv)
{
    if (argc != 2) {
	fprintf(stderr, "usage: filename.wav\n");
	return 1;
    }

    Wav *wav = wav_open(new BufferFile(argv[1]));
    if (wav) {
        TalkingSkullAudioOps *ops = new TalkingSkullAudioOps(wav->to_audio_buffer());
        talking_skull_ops_to_file(stdout, ops);
    }

    return 0;
}
