#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#include "audio.h"
#include "audio-buffer.h"
#include "pi-threads.h"

#include "consoles.h"

class AudioPlayer : PiThread {
public:
    AudioPlayer(Audio *audio);
    ~AudioPlayer();

    bool play(AudioBuffer *audio_buffer);
    void stop();
    void wait_all_done();
    bool wait_current_done(const us_time_t *abstime = NULL);

    bool is_active() { return player_is_active; }
    bool play_sync(AudioBuffer *audio_buffer) {
	if (! play(audio_buffer)) return false;
	us_time_t abstime;
	us_gettime(&abstime);
	us_add_ms(&abstime, audio_buffer->get_duration_ms());
	us_add_ms(&abstime, 15*1000);
	return wait_current_done(&abstime);
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
