#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "consoles.h"
#include "mem.h"
#include "audio.h"

static inline int flags(bool playback) { return (playback ? PCM_OUT : PCM_IN) | PCM_MONOTONIC; }

AudioPi::AudioPi(bool playback, int card, int device) : card(card), device(device), playback(playback), pcm(NULL) {
    mixer = mixer_open(card);

    out_volume = in_volume = NULL;
    out_switch = in_switch = NULL;

    int n = mixer_get_num_ctls(mixer);
    for (int i = 0; i < n; i++) {
        struct mixer_ctl *ctl = mixer_get_ctl(mixer, i);
	if (ctl) {
	    const char *name = mixer_ctl_get_name(ctl);
	    if (! out_volume && strstr(name, "PCM Playback Volume") != NULL) {
		out_volume = ctl;
	    }
	    if (! out_volume && strstr(name, "Playback Volume") != NULL) {
		out_volume = ctl;
	    }
	    if (! in_volume && strstr(name, "PCM Capture Volume") != NULL) {
		in_volume = ctl;
	    }
	    if (! in_volume && strstr(name, "Capture Volume") != NULL) {
		in_volume = ctl;
	    }
	    if (! out_switch && strstr(name, "PCM Playback Route") != NULL) {
		out_switch = ctl;
	    }
	    if (! out_switch && strstr(name, "Playback Switch") != NULL) {
		out_switch = ctl;
	    }
	    if (! in_switch && strstr(name, "Capture Switch") != NULL) {
		out_switch = ctl;
	    }
	}
    }
}

void AudioPi::configure(AudioConfig *new_config) {
    config.channels = new_config->get_num_channels();
    config.rate = new_config->get_rate();
    config.format = new_config->get_bytes_per_sample() == 2 ? PCM_FORMAT_S16_LE : new_config->get_bytes_per_sample() == 4 ? PCM_FORMAT_S32_LE : PCM_FORMAT_S24_LE;
    config.period_size = 1024;
    config.period_count = 2;
    config.silence_threshold = config.period_size * config.period_count;
    config.silence_size = 0;
    config.stop_threshold = config.period_size * config.period_count;
    config.start_threshold = config.period_size;

    if (pcm) pcm_close(pcm);

    if ((pcm = pcm_open(card, device, flags(playback), &config)) == NULL ||
	! pcm_is_ready(pcm))
    {
	consoles_fatal_printf("Failed to open (%d, %d): %s\n", card, device, pcm_get_error(pcm));
	/* TODO: Add -1 for a card and poll until we find a sound device.
	 * This is convenient for the pi5 which doesn't have a headphone jack.
	 */
    }
}

size_t AudioPi::get_recommended_buffer_size() {
    return pcm_frames_to_bytes(pcm, config.period_size);
}

bool AudioPi::play(void *buffer, size_t size) {
    return pcm_writei(pcm, buffer, pcm_bytes_to_frames(pcm, size)) == 0;
}
