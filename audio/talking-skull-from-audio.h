#ifndef __TALKING_SKULL_FROM_AUDIO_H__
#define __TALKING_SKULL_FROM_AUDIO_H__

#include "audio-buffer.h"
#include "talking-skull.h"
#include "wav.h"

class TalkingSkullAudioOps : public TalkingSkullOps {
public:
    TalkingSkullAudioOps(AudioBuffer *audio, unsigned n_to_avg = 1);

    ~TalkingSkullAudioOps() {
	if (delete_audio_buffer) delete audio_buffer;
    }

    int get_usec_per_i() override { return usec_per_i; };
    bool next(double *pos) override;
    bool reset() override;

    static TalkingSkullAudioOps *open_wav(const char *fname, unsigned n_to_avg = 1) {
	AudioBuffer *wav = wav_open(fname);
	if (! wav) return NULL;
	TalkingSkullAudioOps *tsao = new TalkingSkullAudioOps(wav, n_to_avg);
	tsao->delete_audio_buffer = true;
	return tsao;
    }

private:
    AudioBuffer *audio_buffer;
    bool delete_audio_buffer = false;
    unsigned n_per_sample;
    unsigned n_to_avg;
    unsigned last_usec;
    double usec_per_i;
};

#endif
