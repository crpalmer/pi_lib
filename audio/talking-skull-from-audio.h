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

private:
    AudioBuffer *audio;
    unsigned n_per_sample;
    unsigned bytes_per_audio;
    unsigned n_to_avg;
    unsigned last_usec;
    double usec_per_i;
};

class TalkingSkullWavOps : public TalkingSkullOps {
public:
    TalkingSkullWavOps(const char *fname) : wav(new Wav(fname)) {
	audio_buffer = wav->to_audio_buffer();
	skull = new TalkingSkullAudioOps(audio_buffer, true);
    }

    ~TalkingSkullWavOps() {
	delete audio_buffer;
	delete wav;
	delete skull;
    }


    int get_usec_per_i() override {
	return skull->get_usec_per_i();
    }

    bool next(double *pos) override {
	return skull->next(pos);
    }

private:
    Wav *wav;
    TalkingSkullAudioOps *skull;
    AudioBuffer *audio_buffer;
};

#endif
