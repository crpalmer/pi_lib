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

typedef struct {
    unsigned n_per_servo;
    unsigned mx;
    unsigned long long sum;
    unsigned n_samples;
    unsigned max_possible;
    unsigned n_to_avg;
    unsigned last_usec;
    unsigned last_printed_usec;
    unsigned long long i;
    double i_to_usec;
} state_t;

typedef struct {
    unsigned	usec;
    double	pos;
} servo_data_t;

struct talking_skullS {
    audio_meta_t m;
    size_t n_servo, a_servo;
    servo_data_t *servo;
    state_t state;
    talking_skull_servo_update_t fn;
    void *fn_data;
    pthread_t thread;
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

static void
state_init(state_t *s, audio_meta_t *m, bool is_track)
{
    s->n_per_servo = m->sample_rate * m->num_channels / N_SERVO_PER_S;
    s->mx = 0;
    s->sum = 0;
    s->n_samples = 0;
    s->max_possible = (((unsigned) 1)<<(m->bytes_per_sample*8-1))-1;
    s->n_to_avg = is_track ? N_TO_AVG_TRACK : N_TO_AVG_GENERATED;
    s->last_usec = 0;
    s->last_printed_usec = 0;
    s->i = 0;
    s->i_to_usec = 1000.0 * 1000 / m->sample_rate / m->num_channels;
} 

static void
state_update(talking_skull_t *t, unsigned val)
{
    state_t *s = &t->state;

    s->i++;

    if (val > s->mx) s->mx = val;

    if (++s->n_samples % s->n_per_servo == 0) {
	s->sum += s->mx;
	s->mx = 0;
    }

    if (s->n_samples % (s->n_per_servo * s->n_to_avg) == 0) {
	unsigned this_usec = s->i * s->i_to_usec;

	if (t->n_servo >= t->a_servo) {
	    t->a_servo *= 2;
	    t->servo = fatal_realloc(t->servo, sizeof(*t->servo)*t->a_servo);
	}

	t->servo[t->n_servo].pos = ((double) s->sum) / s->n_to_avg / s->max_possible * 100;
	t->servo[t->n_servo].usec = (this_usec - s->last_usec) / 2 + s->last_usec;
	s->last_usec = this_usec;

	if (PRINT_SERVO) {
	    if (t->servo[t->n_servo].usec - s->last_printed_usec > 20*1000) {
		unsigned this_val = t->servo[t->n_servo].pos / 2 + 0.5;
		printf("%9.6f:%*c%*c\n", t->servo[t->n_servo].usec / (1000.0*1000.0), this_val+1, '*', 50 - this_val, '|');
		s->last_printed_usec = t->servo[t->n_servo].usec;
	    }
	}

	t->n_servo++;
	s->n_samples = 0;
	s->sum = 0;
    }
}

talking_skull_t *
talking_skull_new(audio_meta_t *m, bool is_track, talking_skull_servo_update_t fn, void *fn_data)
{
    talking_skull_t *t;

    t = fatal_malloc(sizeof(*t));
    t->m = *m;
    t->n_servo = 0;
    t->a_servo = 1024;
    t->servo = fatal_malloc(sizeof(*t->servo) * t->a_servo);

    if (is_track) {
	t->m.num_channels = 1;
    }

    state_init(&t->state, &t->m, is_track);
    t->fn = fn;
    t->fn_data = fn_data;

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
talking_skull_play(talking_skull_t *t, unsigned char *data, unsigned n_bytes)
{
    size_t i;

    for (i = 0; i < n_bytes; ) {
	unsigned val = decode_value(&t->m, data, &i, t->state.max_possible);
	state_update(t, val);
    }

    pthread_create(&t->thread, NULL, update_main, t);
}

void
talking_skull_wait_completion(talking_skull_t *t)
{
    pthread_join(t->thread, NULL);
}
