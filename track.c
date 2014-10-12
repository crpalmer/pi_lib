#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "audio.h"
#include "mem.h"
#include "wav.h"

#include "track.h"

struct trackS {
    wav_t         *wav;
    audio_device_t audio_dev;
    audio_config_t audio_cfg;
};

track_t *
track_new(const char *fname)
{
    track_t *t;
    wav_t *wav;

    if ((wav = wav_new(fname)) == NULL) {
	return NULL;
    }

    t = fatal_malloc(sizeof(*t));
    t->wav = wav;

    audio_device_init_playback(&t->audio_dev);
    audio_config_init_default(&t->audio_cfg);
    wav_configure_audio(t->wav, &t->audio_cfg);

    return t;
}

void
track_play(track_t *t)
{
    audio_t *audio = audio_new(&t->audio_cfg, &t->audio_dev);
    audio_set_volume(audio, 100);
    wav_play(t->wav, audio);
    audio_destroy(audio);
}

void
track_destroy(track_t *t)
{
    wav_destroy(t->wav);
}
