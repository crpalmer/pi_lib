#include <stdio.h>
#include "pi.h"
#include "consoles.h"
#include "mem.h"
#include "wav.h"

#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

struct riff_wave_header {
    uint32_t riff_id;
    uint32_t riff_sz;
    uint32_t wave_id;
};

struct chunk_header {
    uint32_t id;
    uint32_t sz;
};

struct chunk_fmt {
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

AudioBuffer *wav_open(Buffer *b) {
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt fmt;
    bool more_chunks = true;
    Buffer *audio_data = NULL;

    b->read(&riff_wave_header, sizeof(riff_wave_header));
    if ((riff_wave_header.riff_id != ID_RIFF) || (riff_wave_header.wave_id != ID_WAVE)) {
        consoles_printf("wav: %s is not a riff/wave file\n", b->get_fname());
	return NULL;
    }

    do {
        b->read(&chunk_header, sizeof(chunk_header));

        switch (chunk_header.id) {
        case ID_FMT:
            b->read(&fmt, sizeof(fmt));
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(fmt)) {
                b->seek_rel(chunk_header.sz - sizeof(fmt));
	    }
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = false;
    	    audio_data = b->get_sub_buffer(chunk_header.sz);
            break;
        default:
            /* Unknown chunk, skip bytes */
            b->seek_rel(chunk_header.sz);
        }
    } while (more_chunks);

    if (! audio_data) {
	consoles_printf("wav: %s did not contain any audio data.\n", b->get_fname());
	return NULL;
    }

    return new AudioBuffer(audio_data, fmt.num_channels, fmt.sample_rate, (fmt.bits_per_sample+7) / 8);
}

AudioBuffer *wav_open(const char *fname) {
    BufferFile *b = buffer_file_open(fname);
    if (! b) return NULL;
    AudioBuffer *wav = wav_open(b);
    delete b;
    return wav;
}
