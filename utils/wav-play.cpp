#include <stdio.h>
#include "pi.h"
#include "audio.h"
#include "audio-player.h"
#include "buffer.h"
#include "fanfare-wav.h"
#include "pi.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wav.h"

void load_and_play(AudioPlayer *player, Buffer *buffer) {
    printf("Playing %s\n", buffer->get_fname());
    Wav *wav = new Wav(buffer);

    player->play(wav->to_audio_buffer());
    player->wait_done();
}

void threads_main(int argc, char **argv) {
#ifdef PLATFORM_pi
    Audio *audio = new AudioPi();
#else
    Audio *audio = new AudioPico();
#endif
    AudioPlayer *player = new AudioPlayer(audio);

    if (argc <= 1) {
	load_and_play(player, new BufferBuffer(fanfare_wav, fanfare_wav_len));
    } else {
	for (int i = 1; i < argc; i++) {
	    load_and_play(player, new BufferFile(argv[i]));
	}
    }
}

int main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
    return 0;
}
