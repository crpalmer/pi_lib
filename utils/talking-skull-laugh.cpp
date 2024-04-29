#include <stdio.h>
#include "audio.h"
#include "audio-player.h"
#include "laugh.h"
#include "servo-gpio.h"
#include "talking-skull.h"
#include "talking-skull-from-audio.h"
#include "time-utils.h"
#include "wav.h"

#ifdef PLATFORM_pico

#include <malloc.h>

uint32_t getTotalHeap(void) {
   extern char __StackLimit, __bss_end__;

   return &__StackLimit  - &__bss_end__;
}

uint32_t getFreeHeap(void) {
   struct mallinfo m = mallinfo();

   return getTotalHeap() - m.uordblks;
}
#endif

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
    Buffer *buffer = new BufferBuffer(laugh_wav, laugh_wav_len);
    Wav *wav = new Wav(buffer);
    AudioBuffer *audio_buffer = wav->to_audio_buffer();

    TalkingSkullOps *ops = new TalkingSkullAudioOps(audio_buffer);
    skull->ops(ops);
    delete ops;

    player->play(audio_buffer);
    skull->play();
    player->wait_done();

    delete buffer;
    delete audio_buffer;
    delete wav;
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
	talk_once(audio, player, skull);
#ifdef PLATFORM_pico
        printf("Heap free: %ld\n", (long) getFreeHeap());
#endif
    }
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
