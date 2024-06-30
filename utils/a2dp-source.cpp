#include "pi.h"
#include "audio-player.h"
#include "bluetooth/bluetooth.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wav.h"
#include "wifi.h"

class AudioHack : public Audio {
public:
    size_t get_recommended_buffer_size() { return 256 * 2 * 2; };
    bool configure(AudioConfig *config) { return true; }
    bool play(void *buf, size_t n) {
	extern void a2dp_play(int16_t *buffer, int n);
	a2dp_play((int16_t *) buf, n / 4);
	return true;
    }

    int get_num_channels() { return 2; }
    int get_rate() { return 44100; }
    int get_bytes_per_sample() { return 4; }
};

void play_wav_main(int argc, char **argv) {
    Audio *audio = new AudioHack();
    AudioPlayer *player = new AudioPlayer(audio);
    extern void a2dp_connect();
    extern void a2dp_set_volume(int);

    while (1) {
	static char buf[1024];
	AudioBuffer *audio_buffer;

	pi_readline(buf, sizeof(buf));

	if (strcmp(buf, "connect") == 0) {
	    a2dp_connect();
	} else if (strncmp(buf, "volume ", 7) == 0) {
	    a2dp_set_volume(atoi(&buf[7]));
	} else if (strcmp(buf, "threads") == 0) {
	    pi_threads_dump_state();
	    printf("%d bytes free.\n", pi_threads_get_free_ram());
	} else {
	    audio_buffer = wav_open(buf);
	    if (! audio_buffer) continue;

	    printf("Playing %s\n", audio_buffer->get_fname());
	    struct timespec start;
	    nano_gettime(&start);
	    player->play(audio_buffer);
	    player->wait_all_done();
	    printf("Done: %d ms\n", nano_elapsed_ms_now(&start));

	    delete audio_buffer;
	    pi_threads_dump_state();
	    printf("%d bytes free.\n", pi_threads_get_free_ram());
	}
    }
}

void thread_main(int argc, char **argv) {
    extern int btstack_setup();

#ifdef WITH_WIFI
    wifi_init(CYW43_HOST_NAME);
    //wifi_wait_for_connection();
#endif

    bluetooth_init();
    btstack_setup();

    play_wav_main(argc, argv);
}

int main(int argc, char **argv) {
    pi_init_with_threads(thread_main, argc, argv);
}
