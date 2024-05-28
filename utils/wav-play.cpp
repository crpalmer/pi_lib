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

void threads_main(int argc, char **argv) {
#ifdef PLATFORM_pi
    Audio *audio = new AudioPi();
#else
    Audio *audio = new AudioPico();
#endif
    AudioPlayer *player = new AudioPlayer(audio);

    while (1) {
	static char buf[1024];
	AudioBuffer *audio_buffer;

	pi_readline(buf, sizeof(buf));
	if (strcmp(buf, "fanfare") == 0) {
	    Buffer *buffer = new BufferBuffer(fanfare_wav, fanfare_wav_len);
	    audio_buffer = wav_open(buffer);
	    delete buffer;
	} else {
	    audio_buffer = wav_open(buf);
	}
	if (! audio_buffer) continue;

	printf("Playing %s\n", audio_buffer->get_fname());
	player->play(audio_buffer);
	player->wait_all_done();

	delete audio_buffer;

	pi_threads_dump_state();
	ms_sleep(1000);
    }
}

int main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
    return 0;
}
