#include <stdio.h>
#include "pi.h"
#include "audio.h"
#include "audio-player.h"
#include "gp-output.h"
#include "laugh.h"
#include "servo-gpio.h"
#include "talking-skull.h"
#include "talking-skull-from-audio.h"
#include "time-utils.h"
#include "wav.h"

class ServoTalkingSkull : public TalkingSkull {
public:
    ServoTalkingSkull(int gpio, int eyes_gpio) : TalkingSkull(), servo(new GpioServo(gpio)) {
	servo->set_is_inverted(true);
	if (eyes_gpio > 0) eyes = new GPOutput(eyes_gpio);
	else eyes = NULL;
    }

    virtual ~ServoTalkingSkull() {
	delete servo;
    }

    void update_pos(double pos) override {
	servo->move_to(pos);
	if (eyes) {
	    bool new_eyes = (pos >= 50);
	    if (new_eyes != last_eyes) {
		last_eyes = new_eyes;
		eyes->set(new_eyes);
	    }
	}
    }

private:
    Servo *servo;
    bool last_eyes = false;
    GPOutput *eyes;
};

void talk_once(Audio *audio, AudioPlayer *player, TalkingSkull *skull, AudioBuffer *audio_buffer) {
    TalkingSkullOps *ops = new TalkingSkullAudioOps(audio_buffer);
    skull->set_ops(ops);
    delete ops;

    player->play(audio_buffer);
    skull->play();
    player->wait_done();
    skull->update_pos(0);
}

void threads_main(int argc, char **argv) {
    Audio *audio;
#ifdef PLATFORM_pico
    audio = new AudioPico();
#else
    audio = new AudioPi();
#endif
    AudioPlayer *player = new AudioPlayer(audio);
    TalkingSkull *skull = new ServoTalkingSkull(22, 26);

    const char *song = argc > 1 ? argv[1] : "laugh.wav";
    printf("Loading %s...\n", song);
    AudioBuffer *audio_buffer = wav_open(song);
    if (! audio_buffer) {
        consoles_printf("COULDN'T OPEN %s\n", song);
        while (1) ms_sleep(1000);
    }

    while (1) {
	ms_sleep(1000);
	printf("Starting to talk\n");
	talk_once(audio, player, skull, audio_buffer);
	pi_threads_dump_state();
	printf("Free ram: %d\n", (int) pi_threads_get_free_ram());
    }
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
