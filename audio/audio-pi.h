#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

#include "tinyalsa/asoundlib.h"

class AudioPi : public Audio {
public:
    AudioPi(bool playback = true, int card = -1, int device = -1);
    bool play(void *buffer, size_t n) override;
    size_t get_recommended_buffer_size() override;
    bool configure(AudioConfig *config) override;

    int get_num_channels() override { return config.channels; }
    int get_rate() override { return config.rate; }
    int get_bytes_per_sample() override {
	switch(config.format) {
	case PCM_FORMAT_S16_LE: return 16;
	case PCM_FORMAT_S24_LE: return 24;
	case PCM_FORMAT_S32_LE: return 32;
	default:
	    return 2;
	}
    }

private:
    int               card, device;
    bool              playback;
    struct pcm       *pcm;
    struct pcm_config config;
    struct mixer     *mixer;
    struct mixer_ctl *out_volume, *in_volume;
    struct mixer_ctl *out_switch, *in_switch;
};

#endif
