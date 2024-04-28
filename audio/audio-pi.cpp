#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "consoles.h"
#include "mem.h"
#include "audio.h"

#define MAX_CARDS	10
#define MAX_DEVICES	10

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

bool AudioPi::configure(AudioConfig *new_config) {
    enum pcm_format format = bytes_to_pcm_format(new_config->get_bytes_per_sample());

    if (pcm &&
	config.channels == (unsigned) new_config->get_num_channels() &&
        config.rate == (unsigned) new_config->get_rate() &&
	config.format == format) {
	/* Config hasn't changed, we can continue to use the existing PCM handle */
	return true;
    }

    config.channels = new_config->get_num_channels();
    config.rate = new_config->get_rate();
    config.format = format;
    config.period_size = 1024;
    config.period_count = 2;
    config.silence_threshold = config.period_size * config.period_count;
    config.silence_size = 0;
    config.stop_threshold = config.period_size * config.period_count;
    config.start_threshold = config.period_size;

    if (pcm) pcm_close(pcm);
    pcm = NULL;

    if (card >= 0 && device >= 0) {
	if ((pcm = pcm_open(card, device, flags(playback), &config)) == NULL) {
	    consoles_printf("Preferred audio device (%d,%d) is not compatible or does not exist.\n", card, device);
	}
    } else if (card >= 0) {
	for (int d = 0; d < MAX_DEVICES && ! pcm; d++) {
	    if ((pcm = pcm_open(card, d, flags(playback), &config)) != NULL) {
		if (pcm_is_ready(pcm)) consoles_printf("Found compatible PCM device (%d, %d)\n", card, d);
		else pcm = NULL;
	    }
	}
    }

    for (int c = 0; c < MAX_CARDS && ! pcm; c++) {
	for (int d = 0; d < MAX_DEVICES && ! pcm; d++) {
	    if ((pcm = pcm_open(c, d, flags(playback), &config)) != NULL) {
		if (pcm_is_ready(pcm)) consoles_printf("Found compatible PCM device (%d, %d)\n", c, d);
		else pcm = NULL;
	    }
	}
    }

    if (! pcm || ! pcm_is_ready(pcm)) {
	consoles_printf("Could not find a PCM device that is compatible.\n");
	return false;
    }

    return true;
}

size_t AudioPi::get_recommended_buffer_size() {
    return pcm_frames_to_bytes(pcm, config.period_size);
}

bool AudioPi::play(void *buffer, size_t size) {
    return pcm_writei(pcm, buffer, pcm_bytes_to_frames(pcm, size)) == 0;
}
