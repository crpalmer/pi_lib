#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "consoles.h"
#include "mem.h"
#include "time-utils.h"

#include "talking-skull-from-audio.h"

#define SAMPLE_MS	20

TalkingSkullAudioOps::TalkingSkullAudioOps(AudioBuffer *audio_buffer, unsigned n_to_avg) : audio_buffer(audio_buffer), n_to_avg(n_to_avg) {
    unsigned long long values_per_sec = audio_buffer->get_rate() / n_to_avg;
    n_per_sample = values_per_sec * SAMPLE_MS / 1000;

    double i_to_usec = 1000.0 * 1000 / audio_buffer->get_rate();
    usec_per_i = i_to_usec * n_per_sample * n_to_avg;

    audio_buffer->reset();
}

bool TalkingSkullAudioOps::next(double *pos) {
    unsigned long long sum = 0;
    for (unsigned i_avg = 0; i_avg < n_to_avg; i_avg++) {
	unsigned mx = 0;
	for (unsigned i_max = 0; i_max < n_per_sample; i_max++) {
	    int16_t l_signed, r_signed;

	    if (! audio_buffer->next(&l_signed, &r_signed)) return false;
	    uint32_t l = l_signed < 0 ? -l_signed : l_signed;
	    uint32_t r = r_signed < 0 ? -r_signed : r_signed;
	    l <<= 16;
	    r <<= 16;
	    uint32_t val = l > r ? l : r;
	    if (val > mx) mx = val;
	}
	sum += mx;
    }
    
    *pos = ((double) sum) / n_to_avg / (0x7fffffff) * 100;
    return true;
}

bool TalkingSkullAudioOps::reset() {
    return audio_buffer->reset();
}

int TalkingSkullAudioOps::get_n_ops() {
    return audio_buffer->get_n_samples() / n_per_sample / n_to_avg;
}
