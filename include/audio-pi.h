#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

typedef struct alsa_dataS alsa_data_t;

class AudioPi : public Audio {
public:
    AudioPi(bool playback = true, int card = -1, int device = -1);
    bool play(void *buffer, size_t n) override;
    size_t get_recommended_buffer_size() override;
    bool configure(AudioConfig *config) override;

    int get_num_channels() override;
    int get_rate() override;
    int get_bytes_per_sample() override;

private:
    int               card, device;
    bool              playback;
    alsa_data_t      *alsa;

    int bytes_to_pcm_format(int bytes);
    int pcm_format_to_bytes(int format);
};

#endif
