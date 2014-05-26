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

wav_t *
wav_new_with_servo_track(const char *fname, wav_servo_update_t fn, void *fn_data)
{
    int i, j, k;
    unsigned n_per_servo;
    unsigned mx = 0;
    unsigned n_samples = 0;

    wav_t *w = wav_new(fname);

    if (! w) return NULL;

    w->fn = fn;
    w->fn_data = fn_data;

    /* Overestimate a little bit so we don't have to worry about edge cases
     * in this computation.
     */

    w->n_servo = (w->n_audio / w->bytes_per_sample / w->fmt.num_channels + 1) * N_SERVO_PER_S;

    /* Let this round down as we will compute the real time as we output samples */
    n_per_servo = w->fmt.sample_rate / N_SERVO_PER_S;

    w->servo = malloc(sizeof(*w->servo) * w->n_audio);

    for (i = j = 0; i < w->n_audio; ) {
	unsigned channel = (i / w->bytes_per_sample) % w->fmt.num_channels;

	if (channel % w->fmt.num_channels < w->fmt.num_channels-1) {
	    w->audio[j++] = w->audio[i++];
	} else {
	    short val = 0;
	    unsigned shift = 0;
	    double max_possible;

	    do {
		val |= (w->audio[i++] << shift);
		shift += 8;
	    } while (i % w->bytes_per_sample);

	    max_possible = (1 << (shift - 1)) - 1;
	    if (val < 0) val = -val;
	    if (val < 0) val = max_possible;
	    if (val > mx) mx = val;
	    if (++n_samples % n_per_servo == 0) {
		w->servo[k].pos = mx / max_possible * 100;
		mx = 0;
		w->servo[k].usec = ((long long) i) / w->bytes_per_sample / w->fmt.num_channels * 1000 * 1000 / w->fmt.sample_rate;
		k++;
		n_samples = 0;
	    }
	}
    }

    assert(k <= w->n_servo);

    w->n_audio = j;
    w->n_servo = 0;

    for (i = 0; i < k; i += N_TO_AVG) {
	double sum = 0;

	for (j = 0; j < N_TO_AVG && i+j < k; j++) {
	    sum += w->servo[i+j].pos;
	}
	
	w->servo[w->n_servo].usec = w->servo[i+j/2].usec;
	w->servo[w->n_servo].pos  = sum / j;
	w->n_servo++;
    }
	
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

printf("start %ld.%09ld\n", start.tv_sec, start.tv_nsec);
printf("end   %ld.%09ld\n", next.tv_sec, next.tv_nsec);

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
