#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "string-utils.h"
#include "tinyalsa/asoundlib.h"
#include "talking-skull.h"
#include "time-utils.h"
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
    audio_meta_t meta;
    size_t n_audio, n_servo;
    unsigned char *audio;
    unsigned char *servo;
};

void stream_close(int sig)
{
    /* allow the stream to be closed gracefully */
    signal(sig, SIG_IGN);
}

#define MEDIA_DIR "/home/crpalmer/halloween-media"

wav_t *
wav_new(const char *fname)
{
    wav_t *w;
    FILE  *f;
    struct riff_wave_header riff_wave_header;
    struct chunk_header chunk_header;
    struct chunk_fmt fmt;
    bool more_chunks = true;

    if ((f = fopen(fname, "rb")) == NULL) {
	char *alt_name = maprintf("%s/%s", MEDIA_DIR, fname);
	f = fopen(alt_name, "rb");
	free(alt_name);
	if (! f) {
	    perror(fname);
	    return NULL;
	}
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
            fread(&fmt, sizeof(fmt), 1, f);
            /* If the format header is larger, skip the rest */
            if (chunk_header.sz > sizeof(fmt))
                fseek(f, chunk_header.sz - sizeof(fmt), SEEK_CUR);
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

    w->meta.sample_rate = fmt.sample_rate;
    w->meta.num_channels = fmt.num_channels;
    w->meta.bytes_per_sample = (fmt.bits_per_sample+7) / 8;

    return w;
}

void
wav_extract_servo_track(wav_t *w)
{
    size_t i, j, k;
    unsigned char *servo_track;
    unsigned servo_channel;

    servo_track = malloc(w->n_audio);
    servo_channel = w->meta.num_channels-1;

    for (i = j = k = 0; i < w->n_audio; i++) {
	unsigned channel = (i / w->meta.bytes_per_sample) % w->meta.num_channels;

	if (channel % w->meta.num_channels == servo_channel) {
	    servo_track[k++] = w->audio[i];
	} else {
	    w->audio[j++] = w->audio[i];
	}
    }

    w->n_audio = j;
    w->meta.num_channels -= 1;

    w->n_servo = k;
    w->servo = servo_track;
}

audio_meta_t
wav_get_meta(wav_t *w)
{
    return w->meta;
}

unsigned char *
wav_get_raw_data(wav_t *w, unsigned *n_bytes)
{
    *n_bytes = w->n_audio;
    return w->audio;
}

void
wav_configure_audio(wav_t *w, audio_config_t *a)
{
    a->rate = w->meta.sample_rate;
    a->channels = w->meta.num_channels;
    a->bits = w->meta.bytes_per_sample * 8;
}

bool
play_internal(wav_t *w, audio_t *audio, talking_skull_t *talking_skull, stop_t *stop)
{
    size_t size;
    size_t i;
    bool rc = true;
    unsigned handle;

    assert(w);
    assert(! talking_skull || ! stop);

    size = audio_get_buffer_size(audio);

    if (talking_skull) {
	if (w->servo) {
	    handle = talking_skull_play(talking_skull, w->servo, w->n_servo);
	} else {
	    handle = talking_skull_play(talking_skull, w->audio, w->n_audio);
	}
    }

    for (i = 0; i < w->n_audio && ! stop_requested(stop); i += size) {
	size_t this_size = i + size > w->n_audio ? w->n_audio - i : size;
	if (! audio_play_buffer(audio, &w->audio[i], this_size)) {
	    perror("Error playing sample");
	    rc = false;
	    break;
	}
    }

    if (talking_skull) {
	talking_skull_wait_completion(talking_skull, handle);
    }

    stop_stopped(stop);

    return rc;
}

bool
wav_play(wav_t *w, audio_t *audio)
{
    return wav_play_with_stop(w, audio, NULL);
}

bool
wav_play_with_stop(wav_t *w, audio_t *audio, stop_t *stop)
{
    return play_internal(w, audio, NULL, stop);
}

bool
wav_play_with_talking_skull(wav_t *w, audio_t *audio, talking_skull_t *talking_skull)
{
    return play_internal(w, audio, talking_skull, NULL);
}

void
wav_destroy(wav_t *w)
{
    assert(w);
    free(w->audio);
    free(w);
}

