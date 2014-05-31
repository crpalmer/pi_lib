#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include "mem.h"
#include "time-utils.h"
#include "talking-skull.h"

#define PRINT_SERVO     1
#define N_SERVO_PER_S   5000
#define N_TO_AVG_TRACK  5
#define N_TO_AVG_GENERATED 250

struct talking_skullS {
    size_t n_audio, n_servo;
    struct {
        unsigned        usec;
        double          pos;
    } *servo;
    pthread_t thread;
    talking_skull_servo_update_t fn;
    void *fn_data;
};

static unsigned
decode_value(audio_meta_t *m, unsigned char *data, size_t *cur, unsigned max_possible)
{
    short val = 0;
    unsigned shift = 0;

    do {
	val |= (data[(*cur)++] << shift);
	shift += 8;
    } while ((*cur) % m->bytes_per_sample);

    if (val < 0) val = -val;
    if (val < 0) val = max_possible;

    return val;
}

talking_skull_t *
talking_skull_new(audio_meta_t *m, bool is_track, unsigned char *data, unsigned n_bytes)
{
    talking_skull_t *t;
    unsigned n_per_servo;
    unsigned mx = 0;
    unsigned long long sum = 0;
    unsigned n_samples = 0;
    unsigned max_possible;
    unsigned n_to_avg = is_track ? N_TO_AVG_TRACK : N_TO_AVG_GENERATED;
    unsigned last_usec = 0;
    size_t n;
    size_t i;

    n_per_servo = m->sample_rate * m->num_channels / N_SERVO_PER_S;
    n = (n_bytes / m->bytes_per_sample / n_per_servo / n_to_avg) + 1;

    t = fatal_malloc(sizeof(*t));
    t->n_servo = 0;
    t->servo = malloc(sizeof(*t->servo) * n);

    max_possible = (((unsigned) 1)<<(m->bytes_per_sample*8-1))-1;

    for (i = 0; i < n_bytes; ) {
	unsigned val = decode_value(m, data, &i, max_possible);

	if (val > mx) mx = val;

	if (++n_samples % n_per_servo == 0) {
	    sum += mx;
	    mx = 0;
	}
	if (n_samples % (n_per_servo * n_to_avg) == 0) {
	    unsigned this_usec = ((long long) i) / m->bytes_per_sample * 1000 * 1000 / m->sample_rate / m->num_channels;
	    t->servo[t->n_servo].pos = ((double) sum) / n_to_avg / max_possible * 100;
	    t->servo[t->n_servo].usec = (this_usec - last_usec) / 2 + last_usec;
	    last_usec = this_usec;

	    if (PRINT_SERVO) {
		unsigned this_val = t->servo[t->n_servo].pos / 2 + 0.5;
	        printf("%9.6f:%*c%*c\n", t->servo[t->n_servo].usec / (1000.0*1000.0), this_val+1, '*', 50 - this_val, '|');
	    }

	    t->n_servo++;
	    n_samples = 0;
	    sum = 0;
	}
    }

    assert(t->n_servo <= n);

    return t;
}


static void *
update_main(void *t_as_vp)
{

    talking_skull_t *t = (talking_skull_t *) t_as_vp;
    struct timespec start, next;
    unsigned cur = 0;

    nano_gettime(&start);
    while (cur < t->n_servo) {
	next = start;
	nano_add_usec(&next, t->servo[cur].usec);
	nano_sleep_until(&next);
	t->fn(t->fn_data, t->servo[cur].pos);
	cur++;
    }

    return NULL;
}

void
talking_skull_play(talking_skull_t *t, talking_skull_servo_update_t fn, void
 *fn_data)
{
    t->fn = fn;
    t->fn_data = fn_data;
    pthread_create(&t->thread, NULL, update_main, t);
}

void
talking_skull_wait_completion(talking_skull_t *t)
{
    pthread_join(t->thread, NULL);
}
