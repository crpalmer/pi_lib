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
	while (player_is_active) stop_cond->wait(mutex);
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

void AudioPlayer::wait_all_done() {
    mutex->lock();
    while (player_is_active) stop_cond->wait(mutex);
    mutex->unlock();
}

bool AudioPlayer::wait_current_done(const us_time_t *abstime) {
    bool ret = true;
    mutex->lock();
    if (player_is_active) ret = stop_cond->wait(mutex, abstime);
    mutex->unlock();
    return ret;
}

void AudioPlayer::main(void) {
    mutex->lock();
    while (1) {
	while (true) {
	    start_cond->wait(mutex);
	    if (buffer) break;
	    stop_requested = false;
	}

	mutex->unlock();

	audio->configure(buffer);
        size_t n = audio->get_recommended_buffer_size();
        uint8_t *buf = (uint8_t *) fatal_malloc(n);

	buffer->reset();

	while (! stop_requested) {
	    size_t bytes = 0;
	    for (bytes = 0; bytes < n; ) {
		int16_t l, r;

		if (! buffer->next(&l, &r)) break;

		/* TODO: Is this the right order: r then l ? */
		buf[bytes++] = r & 0xff;
		buf[bytes++] = r>>8;
		buf[bytes++] = l & 0xff;
		buf[bytes++] = l>>8;
 	    }

	    if (bytes < 0) {
		consoles_printf("audio-player failed to read data, aborting stream.\n");
		break;
	    } else {
		audio->play(buf, bytes);
		if (bytes < n) break;
	    }
	}

	fatal_free(buf);

	mutex->lock();

	audio->disable();
	player_is_active = false;
	stop_requested = false;
	buffer = NULL;

	stop_cond->signal();
    }
}
