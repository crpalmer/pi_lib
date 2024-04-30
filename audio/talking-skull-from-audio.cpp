#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "consoles.h"
#include "mem.h"
#include "time-utils.h"

#include "talking-skull-from-audio.h"

#define SAMPLE_MS	20

TalkingSkullAudioOps::TalkingSkullAudioOps(AudioBuffer *audio, unsigned n_to_avg) : audio(audio), n_to_avg(n_to_avg) {
    unsigned long long values_per_sec = audio->get_rate() * audio->get_num_channels() / n_to_avg;
    n_per_sample = values_per_sec * SAMPLE_MS / 1000;

    bytes_per_audio = audio->get_bytes_per_sample();

    double i_to_usec = 1000.0 * 1000 / audio->get_rate() / audio->get_num_channels();
    usec_per_i = i_to_usec * n_per_sample * n_to_avg;
}

bool TalkingSkullAudioOps::next(double *pos) {
    unsigned long long sum = 0;
    for (unsigned i_avg = 0; i_avg < n_to_avg; i_avg++) {
	unsigned mx = 0;
	for (unsigned i_max = 0; i_max < n_per_sample; i_max++) {
	    uint32_t val_unsigned;
	    int32_t val;

	    if (! audio->next(&val_unsigned)) return false;
	    val = val_unsigned;
	    if (val < 0) val = -val;
	    if ((unsigned) val > mx) mx = val;
	}
	sum += mx;
    }
    
    *pos = ((double) sum) / n_to_avg / (0x7fffffff) * 100;
    return true;
}
