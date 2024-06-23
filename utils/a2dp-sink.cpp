#include "pi.h"
#include "audio-pico.h"
#include "audio-player.h"
#include "buffer.h"
#include "bluetooth/bluetooth.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wifi.h"

int btstack_main(int argc, const char * argv[]);

static Audio *audio;
static AudioConfig *audio_config;
static AudioPlayer *player;
static class PlayBuffer *buffer;
static AudioBuffer *audio_buffer;

class FakeConfig :  public AudioConfig {
public:
    int get_num_channels() override { return 2; }
    int get_rate() override { return 44100; }
    int get_bytes_per_sample() override { return 2; }
};

class PlayBuffer : public Buffer {
public:
    PlayBuffer() {
	lock = new PiMutex();
	reader = new PiCond();
	writer = new PiCond();
    }

    bool is_eof() override { return false; }
    int seek_abs(long pos) override { return -1; }
    int seek_rel(long pos) override { return -1; }
    const char *get_fname() override { return "<play-buffer>"; }
    Buffer *get_sub_buffer(size_t n) override { return NULL; }
    int get_n() override { return 1; }

    size_t read(void *buffer, size_t n) override {
	lock->lock();
	while (to_fill) reader->wait(lock);

	to_fill = (uint8_t *) buffer;
	to_fill_n = n;
	n_filled = 0;

	while (n_filled == 0) {
	    writer->signal();
	    reader->wait(lock);
	}

	n = n_filled;
	n_filled = 0;

	lock->unlock();

	return n;
    }

    void play(uint8_t *data, size_t n) {
	lock->lock();

	while (n) {
	    while (! to_fill) writer->wait(lock);
	    size_t this_n = n > to_fill_n ? to_fill_n : n;

	    memcpy(to_fill, data, this_n);
	    data += this_n;
	    to_fill += this_n;
	    n_filled += this_n;

	    n -= this_n;
	    to_fill_n -= this_n;

	    if (to_fill_n == 0) {
		to_fill = NULL;
		reader->signal();
	    }
	}

	lock->unlock();
    }

private:
    uint8_t *to_fill = NULL;
    size_t to_fill_n, n_filled = 0;
    PiMutex *lock;
    PiCond *reader, *writer;
};

void play_buffer(int16_t *data, int n_bytes) {
    buffer->play((uint8_t *) data, n_bytes);
}

void thread_main(int argc, char **argv) {
    ms_sleep(1000);

#ifdef WITH_WIFI
    wifi_init(CYW43_HOST_NAME);
    //wifi_wait_for_connection();
#endif

    bluetooth_init();

    audio = new AudioPico();
    audio_config = new FakeConfig();
    player = new AudioPlayer(audio);
    buffer = new PlayBuffer();
    audio_buffer = new AudioBuffer(buffer, audio_config);
    player->play(audio_buffer);

    btstack_main(0, NULL);
    pi_threads_dump_state();
    while (1) ms_sleep(1000000);
}

int main(int argc, char **argv) {
    pi_init_with_threads(thread_main, argc, argv);
}
