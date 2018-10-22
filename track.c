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
    audio_device_t dev;

    audio_device_init_playback(&dev);
    return track_new_audio_dev(fname, &dev);
}

track_t *
track_new_audio_dev(const char *fname, audio_device_t *dev)
{
    track_t *t;
    wav_t *wav;

    if ((wav = wav_new(fname)) == NULL) {
	return NULL;
    }

    t = fatal_malloc(sizeof(*t));
    t->audio_dev = *dev;
    t->wav = wav;
    t->volume = 100;

    audio_config_init_default(&t->audio_cfg);
    wav_configure_audio(t->wav, &t->audio_cfg);

    return t;
}

track_t *
track_new_fatal(const char *fname)
{
    track_t *t;

    if ((t = track_new(fname)) == NULL) {
	exit(1);
    }
    return t;
}

track_t *
track_new_audio_dev_fatal(const char *fname, audio_device_t *dev)
{
    track_t *t;

    if ((t = track_new_audio_dev(fname, dev)) == NULL) {
	exit(1);
    }
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
    track_play_with_stop(t, NULL);
}

void
track_play_with_stop(track_t *t, stop_t *stop)
{
    audio_t *audio = audio_new(&t->audio_cfg, &t->audio_dev);
    audio_set_volume(audio, t->volume);
    wav_play_with_stop(t->wav, audio, stop);
    audio_destroy(audio);
}

typedef struct {
    track_t *t;
    stop_t *stop;
} thread_data_t;

static void *
play_main(void *td_as_vp)
{
    thread_data_t *td = (thread_data_t *) td_as_vp;

    track_play_with_stop(td->t, td->stop);
    free(td);
    return NULL;
}

void
track_play_asynchronously(track_t *t, stop_t *stop)
{
    pthread_t thread;
    thread_data_t *td = fatal_malloc(sizeof(*td));

    td->t = t;
    td->stop = stop;

    if (stop) stop_reset(stop);

    pthread_create(&thread, NULL, play_main, td);
    pthread_detach(thread);
}

typedef struct {
    track_t *t;
    stop_t  *stop;
} loop_args_t;

void *
loop_main(void *args_as_vp)
{
    loop_args_t *args = (loop_args_t *) args_as_vp;

    while (! args->stop || ! stop_requested(args->stop)) {
	track_play(args->t);
    }

    stop_stopped(args->stop);
    free(args);

    return NULL;
}

void
track_play_loop(track_t *t, stop_t *stop)
{
    pthread_t thread;

    loop_args_t *args = fatal_malloc(sizeof(*args));
    args->t = t;
    args->stop = stop;
    pthread_create(&thread, NULL, loop_main, args);
}

void
track_destroy(track_t *t)
{
    wav_destroy(t->wav);
}
