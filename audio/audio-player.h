#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#include "audio.h"
#include "audio-buffer.h"
#include "pi-threads.h"

#include "consoles.h"

class AudioPlayer : PiThread {
public:
    AudioPlayer(Audio *audio);

    bool play(AudioBuffer *audio_buffer);
    void stop();
    void wait_done();

    bool is_active() { return player_is_active; }
    bool play_sync(AudioBuffer *audio_buffer) {
	if (! play(audio_buffer)) return false;
	wait_done();
	return true;
    }

    void main(void) override;

private:
    bool         player_is_active = false;
    bool         stop_requested = false;
    Audio       *audio;
    AudioBuffer *buffer = NULL;
    PiMutex     *mutex;
    PiCond      *start_cond, *stop_cond;
};

#endif
