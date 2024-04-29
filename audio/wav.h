#ifndef __WAV_H__
#define __WAV_H__

#include "audio.h"
#include "audio-buffer.h"
#include "buffer.h"

class Wav : public AudioConfig {
public:
    Wav(const char *fname) : Wav(new BufferFile(fname)) { delete_buffer = true; }
    Wav(Buffer *b);
    virtual ~Wav();
    AudioBuffer *to_audio_buffer();

    int get_num_channels() override { return num_channels; }
    int get_rate() override { return sample_rate; }
    int get_bytes_per_sample() override { return bytes_per_sample; }

private:
    uint16_t num_channels, sample_rate, bytes_per_sample;
    Buffer *audio = NULL;
    Buffer *buffer;
    bool delete_buffer = false;
};

#endif
