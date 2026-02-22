#include <stdio.h>
#include "pi.h"
#include "audio.h"
#include "audio-player.h"
#include "buffer.h"
#include "fanfare-wav.h"
#include "file.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wav.h"

Audio *audio;
AudioPlayer *player;

static class WavIterator : public FileForeach {
    bool file(const char *fullname) override {
	AudioBuffer *wav = wav_open(fullname);
	if (wav) {
	    printf("%s - %d ms: %d channels, %d khz, %d bytes per sample\n", fullname, wav->get_duration_ms(), wav->get_num_channels(), wav->get_rate(), wav->get_bytes_per_sample());
	    delete wav;
	}
	return true;
     }
} wav_iterator;

static void play(AudioBuffer *audio_buffer) {
    printf("Playing %s\n", audio_buffer->get_fname());
    us_time_t start;
    us_gettime(&start);
    player->play(audio_buffer);
    player->wait_all_done();
    delete audio_buffer;
    printf("Done: %d ms\n", us_elapsed_ms_now(&start));
}

void threads_main(int argc, char **argv) {
    audio = Audio::create_instance();
    player = new AudioPlayer(audio);

    while (1) {
	static char buf[1024];
	AudioBuffer *audio_buffer;

	pi_readline(buf, sizeof(buf));

	if (strcmp(buf, "help") == 0 || strcmp(buf, "?") == 0) {
	    printf("fanfare - play from a static buffer\n");
	    printf("ls [dir] - list files and their format information\n");
	    printf("<otherwise> - load that file and try to play it\n");
	} else if (strcmp(buf, "threads") == 0) {
	    pi_threads_dump_state();
	} else if (strcmp(buf, "free") == 0) {
	    printf("Free RAM: %ld\n", (long) pi_threads_get_free_ram());
	} else if (strcmp(buf, "fanfare") == 0) {
	    Buffer *buffer = new MemoryBuffer(fanfare_wav, fanfare_wav_len);
	    audio_buffer = wav_open(buffer);
	    delete buffer;
	    play(audio_buffer);
	} else if (strcmp(buf, "ls") == 0) {
	    if (! wav_iterator.foreach()) perror(".");
	} else if (strncmp(buf, "ls ", 3) == 0) {
	    if (! wav_iterator.foreach(&buf[3])) perror(&buf[3]);
	} else {
	    if ((audio_buffer = wav_open(buf)) == NULL) perror(buf);
	    else play(audio_buffer);
	}
    }
}

int main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
    return 0;
}
