#include <stdio.h>
#include "pi.h"
#include "audio.h"
#include "audio-player.h"
#include "laugh.h"
#include "servo-gpio.h"
#include "talking-skull.h"
#include "talking-skull-from-audio.h"
#include "time-utils.h"
#include "wav.h"

class LaughTalkingSkull : public TalkingSkull {
public:
    LaughTalkingSkull(int gpio) : TalkingSkull(), servo(new GpioServo(gpio)) {}

    virtual ~LaughTalkingSkull() {
	delete servo;
    }

    void update_pos(double pos) override {
	servo->move_to(pos);
    }

private:
    Servo *servo;
};

void talk_once(Audio *audio, AudioPlayer *player, TalkingSkull *skull) {
    Buffer *buffer = new MemoryBuffer(laugh_wav, laugh_wav_len);
    AudioBuffer *audio_buffer = wav_open(buffer);
    delete buffer;

    TalkingSkullOps *ops = new TalkingSkullAudioOps(audio_buffer);
    skull->set_ops(ops);
    delete ops;

    skull->play();
    if (! player->play_sync(audio_buffer)) {
	consoles_fatal_printf("fatal: Audio player didn't complete playing the song.\n");
    }

    delete audio_buffer;
}

void threads_main(int argc, char **argv) {
    Audio *audio;
#ifdef PLATFORM_pico
    audio = new AudioPico();
#else
    audio = new AudioPi();
#endif
    AudioPlayer *player = new AudioPlayer(audio);
    TalkingSkull *skull = new LaughTalkingSkull(22);

    printf("Loading...\n");
    while (1) {
	ms_sleep(1000);
	printf("Starting to talk\n");
	talk_once(audio, player, skull);
	pi_threads_dump_state();
    }
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
