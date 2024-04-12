#ifndef _PICO_AUDIO_H
#define _PICO_AUDIO_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

class PicoAudio {
public:
    PicoAudio(PIO pio = pio0, int sm = 0);
    void set_frequency(int freq);
    void play(unsigned char *samples, int n_samples);

private:
    PIO pio;
    int sm;
};

#endif
