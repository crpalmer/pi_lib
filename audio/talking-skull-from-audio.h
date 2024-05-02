#ifndef __TALKING_SKULL_FROM_AUDIO_H__
#define __TALKING_SKULL_FROM_AUDIO_H__

#include "audio-buffer.h"
#include "talking-skull.h"
#include "wav.h"

class TalkingSkullAudioOps : public TalkingSkullOps {
public:
    TalkingSkullAudioOps(AudioBuffer *audio, unsigned n_to_avg = 1);
    int get_usec_per_i() override { return usec_per_i; };
    bool next(double *pos) override;
    bool reset() override;

private:
    AudioBuffer *audio_buffer;
    unsigned n_per_sample;
    unsigned n_to_avg;
    unsigned last_usec;
    double usec_per_i;
};

class TalkingSkullWavOps : public TalkingSkullOps {
public:
    TalkingSkullWavOps(const char *fname) : wav(new Wav(fname)) {
	audio_buffer = wav->to_audio_buffer();
	audio_ops = new TalkingSkullAudioOps(audio_buffer, true);
    }

    ~TalkingSkullWavOps() {
	delete audio_buffer;
	delete wav;
	delete audio_ops;
    }


    int get_usec_per_i() override {
	return audio_ops->get_usec_per_i();
    }

    bool next(double *pos) override {
	return audio_ops->next(pos);
    }

    bool reset() override {
	return audio_ops->reset();
    }

private:
    Wav *wav;
    TalkingSkullAudioOps *audio_ops;
    AudioBuffer *audio_buffer;
};

#endif
