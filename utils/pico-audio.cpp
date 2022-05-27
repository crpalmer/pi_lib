#include <stdio.h>
#include "pi.h"
#include "pico-audio.h"
#include "pico-audio-wav.h"
#include "time-utils.h"

int main(int argc, char **argv)
{
    pi_init();

    PicoAudio *audio = new PicoAudio();
    while (1) {
	struct timespec start;
	unsigned bytes = sizeof(pico_audio_wav_data) / sizeof(pico_audio_wav_data[0]);
	nano_gettime(&start);
	audio->play(pico_audio_wav_data, bytes);
	sleep_ms(5000);
    }
}
