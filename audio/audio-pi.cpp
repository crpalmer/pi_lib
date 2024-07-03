#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tinyalsa/asoundlib.h"
#include "consoles.h"
#include "mem.h"
#include "audio.h"

#define MAX_CARDS      10
#define MAX_DEVICES    10

struct alsa_dataS {
    struct pcm       *pcm;
    struct pcm_config config;
    struct mixer     *mixer;
#ifdef _THIS_IS_CURRENTLY_UNUSED_BUT_LEAVE_IT_HERE_SO_I_DONT_HAVE_TO_FIGURE_IT_OUT_AGAIN
    struct mixer_ctl *out_volume, *in_volume;
    struct mixer_ctl *out_switch, *in_switch;
#endif
};

static inline int flags(bool playback) { return (playback ? PCM_OUT : PCM_IN) | PCM_MONOTONIC; }

Audio *Audio::create_instance() {
    return new AudioPi();
}

AudioPi::AudioPi(bool playback, int card, int device) : card(card), device(device), playback(playback) {
    alsa = (alsa_data_t *) fatal_malloc(sizeof(*alsa));
    alsa->pcm = NULL;
    alsa->mixer = mixer_open(card);

#ifdef _THIS_IS_CURRENTLY_UNUSED_BUT_LEAVE_IT_HERE_SO_I_DONT_HAVE_TO_FIGURE_IT_OUT_AGAIN
    out_volume = in_volume = NULL;
    out_switch = in_switch = NULL;

    int n = mixer_get_num_ctls(alsa->mixer);
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
#endif
}

bool AudioPi::configure(AudioConfig *new_config) {
    enum pcm_format format = (enum pcm_format) bytes_to_pcm_format(new_config->get_bytes_per_sample());

    if (alsa->pcm &&
	alsa->config.channels == (unsigned) new_config->get_num_channels() &&
        alsa->config.rate == (unsigned) new_config->get_rate() &&
	alsa->config.format == format) {
	/* Config hasn't changed, we can continue to use the existing PCM handle */
	return true;
    }

    alsa->config.channels = new_config->get_num_channels();
    alsa->config.rate = new_config->get_rate();
    alsa->config.format = format;
    alsa->config.period_size = 1024;
    alsa->config.period_count = 2;
    alsa->config.silence_threshold = alsa->config.period_size * alsa->config.period_count;
    alsa->config.silence_size = 0;
    alsa->config.stop_threshold = alsa->config.period_size * alsa->config.period_count;
    alsa->config.start_threshold = alsa->config.period_size;

    if (alsa->pcm) pcm_close(alsa->pcm);
    alsa->pcm = NULL;

    if (card >= 0 && device >= 0) {
	if ((alsa->pcm = pcm_open(card, device, flags(playback), &alsa->config)) == NULL) {
	    consoles_printf("Preferred audio device (%d,%d) is not compatible or does not exist.\n", card, device);
	}
    } else if (card >= 0) {
	for (int d = 0; d < MAX_DEVICES && ! alsa->pcm; d++) {
	    if ((alsa->pcm = pcm_open(card, d, flags(playback), &alsa->config)) != NULL) {
		if (pcm_is_ready(alsa->pcm)) consoles_printf("Found compatible PCM device (%d, %d)\n", card, d);
		else alsa->pcm = NULL;
	    }
	}
    }

    for (int c = 0; c < MAX_CARDS && ! alsa->pcm; c++) {
	for (int d = 0; d < MAX_DEVICES && ! alsa->pcm; d++) {
	    if ((alsa->pcm = pcm_open(c, d, flags(playback), &alsa->config)) != NULL) {
		if (pcm_is_ready(alsa->pcm)) consoles_printf("Found compatible PCM device (%d, %d)\n", c, d);
		else alsa->pcm = NULL;
	    }
	}
    }

    if (! alsa->pcm || ! pcm_is_ready(alsa->pcm)) {
	consoles_printf("Could not find a PCM device that is compatible.\n");
	return false;
    }

    return true;
}

size_t AudioPi::get_recommended_buffer_size() {
    return pcm_frames_to_bytes(alsa->pcm, alsa->config.period_size);
}

bool AudioPi::play(void *buffer, size_t size) {
    return pcm_writei(alsa->pcm, buffer, pcm_bytes_to_frames(alsa->pcm, size)) == 0;
}

int AudioPi::get_num_channels() { return alsa->config.channels; }
int AudioPi::get_rate() { return alsa->config.rate; }
int AudioPi::get_bytes_per_sample() { return pcm_format_to_bytes(alsa->config.format); }

int AudioPi::bytes_to_pcm_format(int bytes) {
    switch(bytes) {
    case 2: return PCM_FORMAT_S16_LE;
    case 3: return PCM_FORMAT_S24_LE;
    case 4: return PCM_FORMAT_S32_LE;
    default:
	consoles_fatal_printf("Unsupported bytes per sample %d\n", bytes);
	return PCM_FORMAT_S16_LE;	/* NOT REACHED */
    }
}

int AudioPi::pcm_format_to_bytes(int format) {
    switch(format) {
    case PCM_FORMAT_S16_LE: return 16;
    case PCM_FORMAT_S24_LE: return 24;
    case PCM_FORMAT_S32_LE: return 32;
    default:
	return 2;
    }
}
