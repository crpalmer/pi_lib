#include <stdio.h>
#include "pi.h"
#include "audio.h"
#include "audio-player.h"
#include "buffer.h"
#include "fanfare-wav.h"
#include "laugh.h"
#include "pi.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wav.h"

void load_and_play(AudioPlayer *player, Buffer *buffer) {
    printf("Playing %s\n", buffer->get_fname());
    Wav *wav = new Wav(buffer);

    AudioBuffer *audio_buffer = wav->to_audio_buffer();
    player->play(audio_buffer);
    delete audio_buffer;
    player->wait_done();
    delete wav;
}

void threads_main(int argc, char **argv) {
#ifdef PLATFORM_pi
    Audio *audio = new AudioPi();
#else
    Audio *audio = new AudioPico();
#endif
    AudioPlayer *player = new AudioPlayer(audio);

    while (1) {
        if (argc <= 1) {
	    Buffer *buffer = new BufferBuffer(fanfare_wav, fanfare_wav_len);
	    load_and_play(player, buffer);
	    delete buffer;
        } else {
	    for (int i = 1; i < argc; i++) {
	        load_and_play(player, new BufferFile(argv[i]));
	    }
        }
	ms_sleep(1000);
    }
}

int main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
    return 0;
}
