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
    struct mixer_ctl *out_volume, *out_switch;
    struct mixer_ctl *in_volume, *in_switch;
};

static inline int flags(bool playback) { return (playback ? PCM_OUT : PCM_IN) | PCM_MONOTONIC; }

Audio *Audio::create_instance() {
    return new AudioPi();
}

AudioPi::AudioPi(bool playback, int card, int device) : card(card), device(device), playback(playback) {
    alsa = (alsa_data_t *) fatal_malloc(sizeof(*alsa));
    alsa->pcm = NULL;
    alsa->mixer = NULL;
}

void AudioPi::setup_mixer(int this_card) {
    if (alsa->mixer) mixer_close(alsa->mixer);
    alsa->mixer = mixer_open(this_card);

    alsa->out_volume = alsa->in_volume = NULL;
    alsa->out_switch = alsa->in_switch = NULL;

    int n = mixer_get_num_ctls(alsa->mixer);
    for (int i = 0; i < n; i++) {
        struct mixer_ctl *ctl = mixer_get_ctl(alsa->mixer, i);
	if (ctl) {
	    const char *name = mixer_ctl_get_name(ctl);

	    if (! alsa->out_volume && strstr(name, "PCM Playback Volume") != NULL) {
		alsa->out_volume = ctl;
	    }
	    if (! alsa->out_volume && strstr(name, "Playback Volume") != NULL) {
		alsa->out_volume = ctl;
	    }
	    if (! alsa->in_volume && strstr(name, "PCM Capture Volume") != NULL) {
		alsa->in_volume = ctl;
	    }
	    if (! alsa->in_volume && strstr(name, "Capture Volume") != NULL) {
		alsa->in_volume = ctl;
	    }
	    if (! alsa->out_switch && strstr(name, "PCM Playback Route") != NULL) {
		alsa->out_switch = ctl;
	    }
	    if (! alsa->out_switch && strstr(name, "Playback Switch") != NULL) {
		alsa->out_switch = ctl;
	    }
	    if (! alsa->in_switch && strstr(name, "Capture Switch") != NULL) {
		alsa->out_switch = ctl;
	    }
	}
    }

    for (unsigned i = 0; i < mixer_ctl_get_num_values(alsa->out_volume); i++) {
        mixer_ctl_set_percent(alsa->out_volume, i, 100);
    }
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
	} else {
	    setup_mixer(card);
	}
    } else if (card >= 0) {
	for (int d = 0; d < MAX_DEVICES && ! alsa->pcm; d++) {
	    if ((alsa->pcm = pcm_open(card, d, flags(playback), &alsa->config)) != NULL) {
		if (pcm_is_ready(alsa->pcm)) {
		    consoles_printf("Found compatible PCM device (%d, %d)\n", card, d);
		    setup_mixer(card);
		}
		else alsa->pcm = NULL;
	    }
	}
    }

    for (int c = 0; c < MAX_CARDS && ! alsa->pcm; c++) {
	for (int d = 0; d < MAX_DEVICES && ! alsa->pcm; d++) {
	    if ((alsa->pcm = pcm_open(c, d, flags(playback), &alsa->config)) != NULL) {
		if (pcm_is_ready(alsa->pcm)) {
		    consoles_printf("Found compatible PCM device (%d, %d)\n", c, d);
		    setup_mixer(c);
		} else alsa->pcm = NULL;
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
