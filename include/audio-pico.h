#ifndef __AUDIO_PICO_H__
#define __AUDIO_PICO_H__

#include "audio.h"

class AudioPico : public Audio {
public:
    /* Connect: VIN to +3.3V
                GND and SCK to GND
                LCK to 14 (SCK)
                DIN to 15 (TX)
                BCK to 13 (CS)
     */
    AudioPico(int pio = 0, int data_pin = 15, int clock_pin_base = 13);
    bool configure(AudioConfig *config) override;
    size_t get_recommended_buffer_size() override;
    bool play(void *buffer, size_t n) override;

    void disable() override;

    int get_num_channels() override { return config ? config->get_num_channels() : 2; }
    int get_rate() override { return config ? config->get_rate() : 44100; }
    int get_bytes_per_sample() override { return config ? config->get_bytes_per_sample() : 2; }

private:
    void config_clocks();

private:
    static const int n_buffers = 3;

    AudioConfig *config = NULL;
    int bytes_per_sample;
    uint8_t *buffers[n_buffers];
    int next_buffer = 0;

    int pio;
    uint8_t sm;
};

#endif
