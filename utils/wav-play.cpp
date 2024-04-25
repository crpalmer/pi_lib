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

class MainThread : public PiThread {
public:
    MainThread(int n_files = 0, char **files = NULL) : n_files(n_files), files(files), PiThread("main") { 
#ifdef PLATFORM_pi
	audio = new AudioPi();
#else
	audio = new AudioPico();
#endif
	player = new AudioPlayer(audio);

	start();
    }

    void main(void) {
	if (n_files == 0) {
	    load_and_play(new BufferBuffer(fanfare_wav, fanfare_wav_len));
	} else {
	    for (int i = 0; i < n_files; i++) {
		load_and_play(new BufferFile(files[i]));
	    }
	}
	printf("All Done.\n");
	pi_reboot_bootloader();
    }

private:
    void load_and_play(Buffer *buffer) {
	printf("Loading %s\n", buffer->get_fname());
	Wav *wav = new Wav(buffer);

	printf("Playing\n");
	player->play(wav->to_audio_buffer());
	player->wait_done();
	printf("Complete.\n");
    }

private:
    int n_files;
    char **files;

    Audio *audio;
    AudioPlayer *player;
};

int main(int argc, char **argv)
{
    pi_init_with_threads();
    new MainThread(
#ifdef PLATFORM_pi
        argc-1, &argv[1]
#endif
    );
    pi_threads_start_and_wait();

    return 0;
}
