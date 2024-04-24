#ifndef __AUDIO_CONFIG_H__
#define __AUDIO_CONFIG_H__

#include "tinyalsa/asoundlib.h"

class AudioPi : public Audio {
public:
    AudioPi(bool playback = true, int card = 0, int device = 0);
    bool play(void *buffer, size_t n) override;
    size_t get_recommended_buffer_size() override;
    void configure(AudioConfig *config) override;

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
