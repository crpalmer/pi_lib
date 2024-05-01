#ifndef __AUDIO_BUFFER_H__
#define __AUDIO_BUFFER_H__

class AudioBuffer;

#include <stdint.h>
#include "audio.h"
#include "buffer.h"
#include "mem.h"

class AudioBuffer : public AudioConfig {
public:
    AudioBuffer(Buffer *buffer, AudioConfig *config, int a_block = 4096) : buffer(buffer), config(config), a_block(a_block) {
	bytes_per_sample = config->get_bytes_per_sample();

	block = (uint8_t *) fatal_malloc(a_block);
	block_pos = 0;
	n_block = 0;
    }

    virtual ~AudioBuffer() {
	free(block);
    }

    bool next(uint32_t *val) {
	*val = 0;
	for (int i = 0; i < bytes_per_sample; i++) {
	    if (block_pos >= n_block) {
		if ((n_block = buffer->read(block, a_block)) <= 0) return false;
		block_pos = 0;
	    }
	    uint8_t byte = block[block_pos++];
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
    AudioConfig *config;

    uint8_t *block;
    int a_block, n_block, block_pos;
    int bytes_per_sample;
};

#endif
