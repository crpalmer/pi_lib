#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

#include "consoles.h"
#include "tinyalsa/asoundlib.h"

class AudioPi : public Audio {
public:
    AudioPi(bool playback = true, int card = -1, int device = -1);
    bool play(void *buffer, size_t n) override;
    size_t get_recommended_buffer_size() override;
    bool configure(AudioConfig *config) override;

    int get_num_channels() override { return config.channels; }
    int get_rate() override { return config.rate; }
    int get_bytes_per_sample() override { return pcm_format_to_bytes(config.format); }

private:
    int               card, device;
    bool              playback;
    struct pcm       *pcm;
    struct pcm_config config;
    struct mixer     *mixer;
    struct mixer_ctl *out_volume, *in_volume;
    struct mixer_ctl *out_switch, *in_switch;

    enum pcm_format bytes_to_pcm_format(int bytes) {
	switch(bytes) {
	case 2: return PCM_FORMAT_S16_LE;
	case 3: return PCM_FORMAT_S24_LE;
	case 4: return PCM_FORMAT_S32_LE;
	default:
	    consoles_fatal_printf("Unsupported bytes per sample %d\n", bytes);
	    return PCM_FORMAT_S16_LE;	/* NOT REACHED */
	}
    }

    int pcm_format_to_bytes(enum pcm_format format) {
	switch(format) {
	case PCM_FORMAT_S16_LE: return 16;
	case PCM_FORMAT_S24_LE: return 24;
	case PCM_FORMAT_S32_LE: return 32;
	default:
	    return 2;
	}
    }
};

#endif
