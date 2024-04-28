#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "audio.h"
#include "audio-buffer.h"
#include "consoles.h"

#include "util.h"

#include "audio-pico.h"

#define SAMPLES_PER_BUFFER 256

AudioPico::AudioPico(int data_pin, int clock_pin_base, int dma_channel, int sm) {
    static audio_format_t audio_format = {
            .sample_freq = 44100,
            .format = AUDIO_BUFFER_FORMAT_PCM_S16,
            .channel_count = 2,
    };

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 4,
    };

    bytes_per_sample = 2 /* channels */ * 2 /* 16 bit signed format */;

    producer_pool = audio_new_producer_pool(&producer_format, 3,
                                                                      SAMPLES_PER_BUFFER); // todo correct size
    bool __unused ok;
    const struct audio_format *output_format;
    struct audio_i2s_config config = {
            .data_pin = (uint8_t) data_pin,
            .clock_pin_base = (uint8_t) clock_pin_base,
            .dma_channel = (uint8_t) dma_channel,
            .pio_sm = (uint8_t) sm,
    };

    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
}

bool AudioPico::configure(AudioConfig *config) {
    if (config->get_num_channels() != 2 || config->get_rate() != 44100 || config->get_bytes_per_sample() != 2) {
	consoles_fatal_printf("Audio format is not currently compatible.  Must be 2 channels, 44100 hz, 16 bit signed.\n");
    }
    this->config = config;
    bytes_per_sample = config->get_num_channels() * config->get_bytes_per_sample();
    return true;
}

size_t AudioPico::get_recommended_buffer_size() {
    return SAMPLES_PER_BUFFER * get_bytes_per_sample() * get_num_channels();
};

bool AudioPico::play(void *data_vp, size_t n) {
    uint8_t *data = (uint8_t *) data_vp;

    for (size_t i = 0; i < n; ) {
        struct audio_buffer *buffer = take_audio_buffer(producer_pool, true);
        uint8_t *bytes = (uint8_t *) buffer->buffer->bytes;

	size_t bytes_per_buffer = buffer->max_sample_count * bytes_per_sample;
	size_t n_to_copy = bytes_per_buffer > (n - i) ? (n - i) : bytes_per_buffer;

	memcpy(bytes, &data[i], n_to_copy);

        buffer->sample_count = n_to_copy / bytes_per_sample;
        give_audio_buffer(producer_pool, buffer);

	i += n_to_copy;
    }

    return true;
}
