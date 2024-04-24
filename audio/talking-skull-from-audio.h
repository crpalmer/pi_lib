#ifndef __TALKING_SKULL_FROM_AUDIO_H__
#define __TALKING_SKULL_FROM_AUDIO_H__

#include "audio-buffer.h"
#include "talking-skull.h"
#include "wav.h"

class TalkingSkullAudioOps : public TalkingSkullOps {
public:
    TalkingSkullAudioOps(const char *wav_fname) : TalkingSkullAudioOps((new Wav(wav_fname))->to_audio_buffer()) {}
    TalkingSkullAudioOps(AudioBuffer *audio, unsigned n_to_avg = 1);
    int get_usec_per_i() override { return usec_per_i; };
    bool next(double *pos) override;

private:
    AudioBuffer *audio;
    unsigned n_per_sample;
    unsigned bytes_per_audio;
    unsigned n_to_avg;
    unsigned last_usec;
    double usec_per_i;
};

#endif
