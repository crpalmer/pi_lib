#ifndef __AUDIO_BUFFER_H__
#define __AUDIO_BUFFER_H__

class AudioBuffer;

#include <stdint.h>
#include "audio.h"
#include "buffer.h"

class AudioBuffer : public AudioConfig {
public:
    AudioBuffer(Buffer *buffer, AudioConfig *config) : buffer(buffer), config(config) {
	bytes_per_sample = config->get_bytes_per_sample();
    }

    bool next(uint32_t *val) {
	*val = 0;
	for (int i = 0; i < bytes_per_sample; i++) {
	    if (buffer->is_eof()) return false;
	    uint8_t byte = buffer->next();
	    *val = *val | (byte << 8*i);
	}
	*val = *val << (8 * (4 - bytes_per_sample));
	return true;
    }

    int get_num_channels() override { return config->get_num_channels(); }
    int get_rate() override { return config->get_rate(); }
    int get_bytes_per_sample() { return config->get_bytes_per_sample(); }

    Buffer *get_buffer() { return buffer; }

private:
    Buffer *buffer;
    int bytes_per_sample;
    AudioConfig *config;
};

#endif
	
