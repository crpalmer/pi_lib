#ifndef __AUDIO_PICO_H__
#define __AUDIO_PICO_H__

class AudioPico : public Audio {
public:
    AudioPico(int data_pin = 18, int clock_pin = 16, int pio = 0, int sm = 0);
    void configure(AudioConfig *config) { this->config = config; }
    void set_frequency(int freq);
    bool play(void *buffer, size_t n) override;

    int get_num_channels() override { return this->config ? config->get_num_channels() : Audio::get_num_channels(); }
    int get_rate() override { return this->config ? config->get_rate() : Audio::get_rate(); }
    int get_bytes_per_sample() override { return this->config ? config->get_bytes_per_sample() : Audio::get_bytes_per_sample(); }

private:
    AudioConfig *config;
    int pio;
    int sm;
};

#endif
