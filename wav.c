#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "tinyalsa/asoundlib.h"
#include "time-utils.h"
#include "util.h"
#include "wav.h"

#define N_SERVO_PER_S  5000
#define N_TO_AVG       5

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
    struct {
	unsigned	usec;
	double		pos;
    } *servo;
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

static void
generate_servo_data(wav_t *w, unsigned char *data, unsigned n_bytes, wav_servo_update_t fn, void *fn_data)
{
    unsigned n_per_servo;
    unsigned mx = 0;
    unsigned long long sum = 0;
    unsigned n_samples = 0;
    unsigned max_possible;
    size_t n;
    size_t i;

    w->fn = fn;
    w->fn_data = fn_data;
    w->n_servo = 0;

    n_per_servo = w->fmt.sample_rate / N_SERVO_PER_S;
    n = (n_bytes / w->bytes_per_sample / n_per_servo / N_TO_AVG) + 1;
    w->servo = malloc(sizeof(*w->servo) * n);

    max_possible = (((unsigned) 1)<<(w->fmt.bits_per_sample-1))-1;

    for (i = 0; i < n_bytes; ) {
	short val = 0;
	unsigned shift = 0;

	do {
	    val |= (data[i++] << shift);
	    shift += 8;
	} while (i % w->bytes_per_sample);

	if (val < 0) val = -val;
	if (val < 0) val = max_possible;
	if (val > mx) mx = val;

	if (++n_samples % n_per_servo == 0) {
	    sum += mx;
	    mx = 0;
	}
	if (n_samples % (n_per_servo * N_TO_AVG) == 0) {
	    w->servo[w->n_servo].pos = ((double) sum) / N_TO_AVG / max_possible * 100;
	    w->servo[w->n_servo].usec = ((long long) i) / w->bytes_per_sample * 1000 * 1000 / w->fmt.sample_rate;
	    //printf("%9.6f: %*c\n", w->servo[w->n_servo].usec / (1000.0*1000.0), (int) (w->servo[w->n_servo].pos / 2), '*');
	    w->n_servo++;
	    n_samples = 0;
	    sum = 0;
	}
    }

    assert(w->n_servo <= n);
}

wav_t *
wav_new_with_servo_track(const char *fname, wav_servo_update_t fn, void *fn_data)
{
    size_t i, j, k;
    unsigned char *servo_track;
    unsigned servo_channel;

    wav_t *w = wav_new(fname);

    if (! w) return NULL;

    servo_track = malloc(w->n_audio);

    servo_channel = w->fmt.num_channels-1;

    for (i = j = k = 0; i < w->n_audio; i++) {
	unsigned channel = (i / w->bytes_per_sample) % w->fmt.num_channels;

	if (channel % w->fmt.num_channels == servo_channel) {
	    servo_track[k++] = w->audio[i];
	} else {
	    w->audio[j++] = w->audio[i];
	}
    }

    w->n_audio = j;

    w->fmt.num_channels -= 1;
    w->fmt.byte_rate -= w->fmt.sample_rate * w->bytes_per_sample;
    w->fmt.block_align -= w->bytes_per_sample;

    generate_servo_data(w, servo_track, k, fn, fn_data);

    free(servo_track);

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

static void *
update_main(void *w_as_vp)
{

    wav_t *w = (wav_t *) w_as_vp;
    struct timespec start, next;
    unsigned cur = 0;

    nano_gettime(&start);
    while (cur < w->n_servo) {
	next = start;
	nano_add_usec(&next, w->servo[cur].usec);
	nano_sleep_until(&next);
	w->fn(w->fn_data, w->servo[cur].pos);
	cur++;
    }

    return NULL;
}


bool
wav_play(wav_t *w)
{
    struct pcm_config config;
    struct pcm *pcm;
    size_t size;
    size_t i;
    bool rc = true;
    pthread_t thread;

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
    printf("ms per servo = %f\n", 1000.0 / (N_SERVO_PER_S / N_TO_AVG));

    pcm = pcm_open(0, 0, PCM_OUT, &config);
    if (! pcm || ! pcm_is_ready(pcm)) {
	fprintf(stderr, "Unable to open pcm device: %s\n", pcm_get_error(pcm));
	return false;
    }

    size = pcm_frames_to_bytes(pcm, pcm_get_buffer_size(pcm));

    if (w->fn) pthread_create(&thread, NULL, update_main, w);

    for (i = 0; i < w->n_audio; i += size) {
	size_t this_size = i + size > w->n_audio ? w->n_audio - i : size;
	if (pcm_write(pcm, &w->audio[i], this_size)) {
	    fprintf(stderr, "Error playing sample: %s\n", pcm_get_error(pcm));
	    rc = false;
	    break;
	}
    }

    if (w->fn) pthread_join(thread, NULL);

    pcm_close(pcm);

    return rc;
}
