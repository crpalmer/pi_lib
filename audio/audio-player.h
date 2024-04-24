#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#include "audio.h"
#include "audio-buffer.h"
#include "pi-threads.h"

#include "consoles.h"

class AudioPlayer : PiThread {
public:
    AudioPlayer(Audio *audio);

    void play(AudioBuffer *audio_buffer);
    void stop();
    void wait_done();

    bool is_active() { return player_is_active; }
    void play_sync(AudioBuffer *audio_buffer) {
	play(audio_buffer);
	wait_done();
    }

    void main(void) override;

private:
    bool         player_is_active = false;
    bool         stop_requested = false;
    Audio       *audio;
    Buffer      *buffer = NULL;
    PiMutex     *mutex;
    PiCond      *start_cond, *stop_cond;
};

#endif
