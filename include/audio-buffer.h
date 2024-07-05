#ifndef __AUDIO_BUFFER_H__
#define __AUDIO_BUFFER_H__

class AudioBuffer;

#include <stdint.h>
#include "audio.h"
#include "buffer.h"
#include "mem.h"
#include "time-utils.h"

class AudioBuffer : public AudioConfig {
public:
    // NOTE: AudioBuffer takes over ownership of buffer.  You should not acess it after pass
    // it to the constructor and the AudioBuffer takes care of deleting it when done.

    AudioBuffer(Buffer *buffer, AudioConfig *config, int a_block = 1024) : AudioBuffer(buffer, config->get_num_channels(), config->get_rate(), config->get_bytes_per_sample(), a_block) { }

    AudioBuffer(Buffer *buffer, int num_channels, int rate, int bytes_per_sample, int a_block = 1024) : buffer(buffer), num_channels(num_channels), rate(rate), bytes_per_sample(bytes_per_sample), a_block(a_block) {
	block = (uint8_t *) fatal_malloc(a_block * sizeof(*block));
	block_pos = 0;
	n_block = 0;
    }

    virtual ~AudioBuffer() {
	delete buffer;
	fatal_free(block);
    }

    virtual Buffer *get_buffer() { return buffer; }

    virtual const char *get_fname() { return buffer->get_fname(); }

    virtual bool next(int16_t *l, int16_t *r) {
	for (int lr = 0; lr < num_channels; lr++) {
	    uint32_t val = 0;
	    for (int i = 0; i < bytes_per_sample; i++) {
	        if (block_pos >= n_block) {
		    if ((n_block = buffer->read(block, a_block)) <= 0) return false;
		    block_pos = 0;
	        }
	        uint8_t byte = block[block_pos++];
		val = val | (byte << 8*i);
	    }
	    val = val << (8 * (4 - bytes_per_sample));
	    if (lr == 0) *l = *r = val;
	    else *r = val;
	}
	return true;
    }

    virtual bool reset() {
	buffer->seek_abs(0);
	n_block = block_pos = 0;
	return true;
    }

    int get_num_channels() override { return 2; }
    int get_rate() override { return rate; }
    int get_bytes_per_sample() { return 2; }

    int get_n_samples() {
	return buffer->get_n() / num_channels / get_bytes_per_sample();
    }

    unsigned get_duration_ms() {
	int n_samples = get_n_samples();
	int rate = get_rate();
	int sec = n_samples / rate;
	int ms = ((1000*1000 / rate) * (n_samples % rate) + 999) / 1000;

	return sec*1000 + ms;
    }

private:
    Buffer *buffer;
    int num_channels;
    int rate;
    int bytes_per_sample;

    uint8_t *block;
    int a_block, n_block, block_pos;
};

#endif
