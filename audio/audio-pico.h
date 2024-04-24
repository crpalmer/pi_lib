#ifndef __AUDIO_PICO_H__
#define __AUDIO_PICO_H__

class AudioPico : public Audio {
public:
    AudioPico(int data_pin = 18, int clock_pin = 16, int pio = 0, int sm = 0);
    bool configure(AudioConfig *config) { this->config = config; return true; }
    void set_frequency(int freq);
    bool play(void *buffer, size_t n) override;

    int get_num_channels() override { return this->config ? config->get_num_channels() : 2; }
    int get_rate() override { return this->config ? config->get_rate() : 44100; }
    int get_bytes_per_sample() override { return this->config ? config->get_bytes_per_sample() : 2; }

private:
    AudioConfig *config;
    int pio;
    int sm;
};

#endif
