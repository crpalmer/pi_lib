#include "pi.h"
#include "consoles.h"
#include "mem.h"

#include "audio-player.h"

AudioPlayer::AudioPlayer(Audio *audio) : PiThread("audio-player"), audio(audio) {
    mutex = new PiMutex();
    stop_cond = new PiCond();
    start_cond = new PiCond();

    start(2);
}

AudioPlayer::~AudioPlayer() {
    delete mutex;
    delete stop_cond;
    delete start_cond;
}

bool AudioPlayer::play(AudioBuffer *audio_buffer) {
    mutex->lock();
    if (player_is_active) {
	stop_requested = true;
	start_cond->signal();
	while (player_is_active) start_cond->wait(mutex);
    }

    if (! audio->configure(audio_buffer)) return false;

    buffer = audio_buffer;
    player_is_active = true;
    start_cond->signal();
    mutex->unlock();

    return true;
}
    
void AudioPlayer::stop() {
    mutex->lock();
    stop_requested = true;
    start_cond->signal();
    mutex->unlock();
}

void AudioPlayer::wait_done() {
    mutex->lock();
    while (player_is_active) stop_cond->wait(mutex);
    mutex->unlock();
}

void AudioPlayer::main(void) {
    mutex->lock();
    while (1) {
	while (! buffer) {
	    start_cond->wait(mutex);
	}

        size_t n = audio->get_recommended_buffer_size();
        void *buf = fatal_malloc(n);

	buffer->reset();
	Buffer *b = buffer->get_buffer();

	while (! b->is_eof() && ! stop_requested) {
	    size_t bytes = b->read(buf, n);
	    if (bytes < 0) {
		consoles_printf("audio-player failed to read data, aborting stream.\n");
		break;
	    } else {
		mutex->unlock();
		audio->play(buf, bytes);
		mutex->lock();
	    }
	}

	fatal_free(buf);

	player_is_active = false;
	stop_requested = false;
	buffer = NULL;

	stop_cond->signal();
    }
}
