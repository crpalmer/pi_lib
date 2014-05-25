#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include "tinyalsa/asoundlib.h"
#include "util.h"
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

struct wavS {
    wav_servo_update_t fn;
    void *fn_data;
    struct chunk_fmt fmt;
    unsigned bytes_per_sample;
    size_t n_audio, n_servo;
    unsigned char *audio;
    unsigned char *servo;
};

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
}

wav_t *
wav_new(const char *fname)
{
    wav_t *w;
    FILE  *f;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    bool more_chunks = true;

    if ((f = fopen(fname, "rb")) == NULL) {
	perror(fname);
	return NULL;
    }

    fread(&riff_wave_header, sizeof(riff_wave_header), 1, f);
    if ((riff_wave_header.riff_id != ID_RIFF) ||
        (riff_wave_header.wave_id != ID_WAVE)) {
        fprintf(stderr, "Error: '%s' is not a riff/wave file\n", fname);
        fclose(f);
        return NULL;
    }

    w = calloc(sizeof(*w), 1);

    do {
        fread(&chunk_header, sizeof(chunk_header), 1, f);

        switch (chunk_header.id) {
        case ID_FMT:
            fread(&w->fmt, sizeof(w->fmt), 1, f);
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(w->fmt))
                fseek(f, chunk_header.sz - sizeof(w->fmt), SEEK_CUR);
            break;
        case ID_DATA:
            /* Stop looking for chunks */
            more_chunks = false;
	    w->n_audio = chunk_header.sz;
	    w->audio = malloc(w->n_audio);
	    fread(w->audio, w->n_audio, 1, f);
            break;
        default:
            /* Unknown chunk, skip bytes */
            fseek(f, chunk_header.sz, SEEK_CUR);
        }
    } while (more_chunks);

    w->bytes_per_sample = (w->fmt.bits_per_sample+7) / 8;

    return w;
}

wav_t *
wav_new_with_servo_track(const char *fname, wav_servo_update_t fn, void *fn_data)
{
    wav_t *w = wav_new(fname);

    if (! w) return NULL;

    w->fn = fn;
    w->fn_data = fn_data;
    w->fmt.num_channels -= 1;
    w->fmt.byte_rate -= w->fmt.sample_rate * w->bytes_per_sample;
    w->fmt.block_align -= w->bytes_per_sample;

    return w;
}

static bool
mixer_set(struct mixer *mixer, const char *name, int value)
{
    struct mixer_ctl *ctl;

    ctl = mixer_get_ctl_by_name(mixer, name);
    if (! ctl) {
	fprintf(stderr, "Failed to find control: %s\n", name);
	mixer_close(mixer);
	return false;
    }

    mixer_ctl_set_value(ctl, 0, value);

    return true;
}

bool
wav_set_volume(unsigned volume)
{
    struct mixer *mixer;

    mixer = mixer_open(0);
    if (! mixer) {
	fprintf(stderr, "Failed to open mixer\n");
	return false;
    }

    if (! mixer_set(mixer, "PCM Playback Volume", volume) ||
	! mixer_set(mixer, "PCM Playback Route", 1))
    {
	mixer_close(mixer);
	return false;
    }

    mixer_close(mixer);

    return true;
}

bool
wav_play(wav_t *w)
{
    struct pcm_config config;
    struct pcm *pcm;
    size_t size;
    size_t i;
    bool rc = true;

    config.channels = w->fmt.num_channels;
    config.rate = w->fmt.sample_rate;
    config.period_size = 1024;
    config.period_count = 4;
    switch(w->fmt.bits_per_sample) {
    case 32: config.format = PCM_FORMAT_S32_LE; break;
    case 16: config.format = PCM_FORMAT_S16_LE; break;
    case  8: config.format = PCM_FORMAT_S8; break;
    }
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;

    printf("audio_format = %u\n", w->fmt.audio_format);
    printf("num_channels = %u\n", w->fmt.num_channels);
    printf("sample_rate = %u\n", w->fmt.sample_rate);
    printf("byte_rate = %u\n", w->fmt.byte_rate);
    printf("block_align = %u\n", w->fmt.block_align);
    printf("bits_per_sample = %u\n", w->fmt.bits_per_sample);

    pcm = pcm_open(0, 0, PCM_OUT, &config);
    if (! pcm || ! pcm_is_ready(pcm)) {
	fprintf(stderr, "Unable to open pcm device: %s\n", pcm_get_error(pcm));
	return false;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));

    for (i = 0; i < w->n_audio; i += size) {
	size_t this_size = i + size > w->n_audio ? w->n_audio - i : size;
	if (pcm_write(pcm, &w->audio[i], this_size)) {
	    fprintf(stderr, "Error playing sample: %s\n", pcm_get_error(pcm));
	    rc = false;
	    break;
	}
    }

    pcm_close(pcm);

    return rc;
}
