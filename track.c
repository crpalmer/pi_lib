#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "audio.h"
#include "mem.h"
#include "wav.h"

#include "track.h"

struct trackS {
    wav_t         *wav;
    audio_device_t audio_dev;
    audio_config_t audio_cfg;
    unsigned	   volume;
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
    t->volume = 100;

    audio_device_init_playback(&t->audio_dev);
    audio_config_init_default(&t->audio_cfg);
    wav_configure_audio(t->wav, &t->audio_cfg);

    return t;
}

void
track_set_volume(track_t *t, unsigned volume)
{
    t->volume = volume;
}

void
track_play(track_t *t)
{
    audio_t *audio = audio_new(&t->audio_cfg, &t->audio_dev);
    audio_set_volume(audio, t->volume);
    wav_play(t->wav, audio);
    audio_destroy(audio);
}

static void *
play_main(void *t_as_vp)
{
    track_play(t_as_vp);
    return NULL;
}

void
track_play_asynchronously(track_t *t)
{
    pthread_t thread;

    pthread_create(&thread, NULL, play_main, t);
    pthread_detach(thread);
}

void
track_destroy(track_t *t)
{
    wav_destroy(t->wav);
}
