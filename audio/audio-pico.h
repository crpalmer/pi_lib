#ifndef __AUDIO_PICO_H__
#define __AUDIO_PICO_H__

#include "pico/audio_i2s.h"
#include "audio.h"

class AudioPico : public Audio {
public:
    /* Connect: VIN to +3.3V
                GND and SCK to GND
                LCK to 14 (SCK)
                DIN to 12 (RX)
                BCK to 13 (CS)
     */
    AudioPico(int data_pin = 12, int clock_pin_base = 13, int dma_channel = 2, int sm = 0);
    bool configure(AudioConfig *config) override;
    size_t get_recommended_buffer_size() override;
    bool play(void *buffer, size_t n) override;

    int get_num_channels() override { return config ? config->get_num_channels() : 2; }
    int get_rate() override { return config ? config->get_rate() : 44100; }
    int get_bytes_per_sample() override { return config ? config->get_bytes_per_sample() : 2; }

private:
    AudioConfig *config = NULL;
    struct audio_buffer_pool *producer_pool = NULL;
    int bytes_per_sample;
};

#endif
