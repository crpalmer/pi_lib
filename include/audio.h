#ifndef __AUDIO_H__
#define __AUDIO_H__

class AudioConfig {
public:
    virtual ~AudioConfig() { }
    virtual int get_num_channels() = 0;
    virtual int get_rate() = 0;
    virtual int get_bytes_per_sample() = 0;
};

class Audio : public AudioConfig {
public:
    virtual size_t get_recommended_buffer_size() { return 1024; };
    virtual bool configure(AudioConfig *config) = 0;
    virtual bool capture(void *buf, size_t n) { return false; }
    virtual bool play(void *buf, size_t n) { return false; }
    virtual void disable() { }

    int get_num_channels() override = 0;
    int get_rate() override = 0;
    int get_bytes_per_sample() = 0;

    static Audio *create_instance();
};

#ifdef PLATFORM_pico
#include "audio-pico.h"
#else
#include "audio-pi.h"
#endif

#endif
