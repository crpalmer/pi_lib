#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include "mem.h"
#include "producer-consumer.h"
#include "time-utils.h"
#include "talking-skull.h"
#include "wav.h"

#define PRINT_SERVO     	0

/* An audio stream is broken in N_SERVO_POS_PER_S chunks / second.
   Each chunk is then broken down into n_to_avg runs.
   Each run is turned into a value by taking the max value in the run.

   So by default for a servo track we will take the max value each 100ms
   and for a synthesized stream from actual audio will be take the max
   each 10,000'th of a second and average 100 of those values to get one
   value.
 */

#define N_SERVO_POS_PER_S	100
#define N_TO_AVG_TRACK		1
#define N_TO_AVG_GENERATED	100

typedef struct {
    unsigned n_per_sample;
    unsigned mx;
    unsigned long long sum;
    unsigned n_samples;
    unsigned max_possible;
    unsigned n_to_avg;
    unsigned last_usec;
    unsigned long long i;
    double i_to_usec;
} state_t;

typedef struct {
    unsigned	usec;
    double	pos;
} servo_data_t;

struct servo_operationsS {
    size_t n, a;
    servo_data_t *servo;
    int free_on_complete;
};

struct talking_skullS {
    audio_meta_t m;
    state_t state;
    talking_skull_servo_update_t fn;
    void *fn_data;
    pthread_t thread;
    producer_consumer_t *ops_pc;
    unsigned seq_complete;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    servo_operations_t *ops;
};

static servo_operations_t *
servo_operations_new(void)
{
    servo_operations_t *ops = fatal_malloc(sizeof(*ops));

    ops->n = 0;
    ops->a = 1024;
    ops->servo = fatal_malloc(ops->a * sizeof(*ops->servo));
    ops->free_on_complete = 0;

    return ops;
}

static void
servo_operations_add(servo_operations_t *ops, unsigned usec, double pos)
{
    if (ops->a <= ops->n) {
	ops->a *= 2;
	ops->servo = fatal_realloc(ops->servo, ops->a * sizeof(*ops->servo));
    }
    ops->servo[ops->n].usec = usec;
    ops->servo[ops->n].pos  = pos;
    ops->n++;
}

static void
servo_operations_play(servo_operations_t *ops, talking_skull_servo_update_t fn, void *fn_data)
{
    struct timespec start, next;
    unsigned cur = 0;

    nano_gettime(&start);

    while (cur < ops->n) {
	struct timespec now;

	nano_gettime(&now);
	next = start;
	nano_add_usec(&next, ops->servo[cur].usec);
	if (nano_later_than(&next, &now)) {
	    nano_sleep_until(&next);
	    fn(fn_data, ops->servo[cur].pos);
	}
	cur++;
    }
}

static void
servo_operations_destroy(servo_operations_t *ops)
{
    free(ops->servo);
    free(ops);
}

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
    if (val > max_possible) val = max_possible;

    return val;
}

static void
state_init(state_t *s, audio_meta_t *m, size_t n_to_avg)
{
    s->n_per_sample = m->sample_rate * m->num_channels / (N_SERVO_POS_PER_S * n_to_avg);
    s->mx = 0;
    s->sum = 0;
    s->n_samples = 0;
    s->max_possible = (((unsigned) 1)<<(m->bytes_per_sample*8-1))-1;
    s->n_to_avg = n_to_avg;
    s->last_usec = 0;
    s->i = 0;
    s->i_to_usec = 1000.0 * 1000 / m->sample_rate / m->num_channels;
} 

static void
state_start_new_buffer(state_t *s)
{
    s->last_usec = 0;
    s->i = 0;
}

static void
state_update(state_t *s, servo_operations_t *ops, unsigned val)
{
    s->i++;

    if (val > s->mx) s->mx = val;

    if (++s->n_samples % s->n_per_sample == 0) {
	s->sum += s->mx;
	s->mx = 0;
    }

    if (s->n_samples % (s->n_per_sample * s->n_to_avg) == 0) {
	unsigned usec;
	unsigned this_usec = s->i * s->i_to_usec;
	double pos;

	usec = s->last_usec;
	pos = ((double) s->sum) / s->n_to_avg / s->max_possible * 100;
	s->last_usec = this_usec;

	servo_operations_add(ops, usec, pos);

	if (PRINT_SERVO) {
	    unsigned this_val = pos / 2 + 0.5;
	    printf("%9.6f:%*c%*c sum %lld / %d = %.1f\n", usec / (1000.0*1000.0), this_val+1, '*', 50 - this_val, '|', s->sum, s->n_to_avg, ((double) s->sum) / s->n_to_avg);
	}

	s->n_samples = 0;
	s->sum = 0;
    }
}

static void *
update_main(void *t_as_vp)
{
    talking_skull_t *t = (talking_skull_t *) t_as_vp;

    while (true) {
	unsigned seq;
	servo_operations_t *ops;

	ops = producer_consumer_consume(t->ops_pc, &seq);
	servo_operations_play(ops, t->fn, t->fn_data);
	if (ops->free_on_complete) servo_operations_destroy(ops);

	pthread_mutex_lock(&t->mutex);
	t->seq_complete = seq;
	pthread_cond_signal(&t->cond);
	pthread_mutex_unlock(&t->mutex);
    }

    return NULL;
}


talking_skull_t *
talking_skull_new(audio_meta_t *m, bool is_track, talking_skull_servo_update_t fn, void *fn_data)
{
    audio_meta_t local_m;

    if (m) local_m = *m;
    if (is_track) {
	local_m.num_channels = 1;
    }

    return talking_skull_new_with_n_to_avg(m ? &local_m : NULL, is_track ? N_TO_AVG_TRACK : N_TO_AVG_GENERATED, fn, fn_data);
}

talking_skull_t *
talking_skull_new_with_n_to_avg(audio_meta_t *m, size_t n_to_avg, talking_skull_servo_update_t fn, void *fn_data)
{
    talking_skull_t *t;

    t = fatal_malloc(sizeof(*t));

    if (m) t->m = *m;
    else memset(&t->m, 0, sizeof(t->m));

    t->ops = NULL;
    t->ops_pc = producer_consumer_new(1);
    t->seq_complete = 0;
    pthread_mutex_init(&t->mutex, NULL);
    pthread_cond_init(&t->cond, NULL);

    state_init(&t->state, &t->m, n_to_avg);
    t->fn = fn;
    t->fn_data = fn_data;

    pthread_create(&t->thread, NULL, update_main, t);

    return t;
}

servo_operations_t *
talking_skull_prepare(talking_skull_t *t, unsigned char *data, unsigned n_bytes)
{
    size_t i;
    servo_operations_t *ops;

    ops = servo_operations_new();

    state_start_new_buffer(&t->state);

    for (i = 0; i < n_bytes; ) {
	unsigned val = decode_value(&t->m, data, &i, t->state.max_possible);
	state_update(&t->state, ops, val);
    }

    return ops;
}

unsigned
talking_skull_play_prepared(talking_skull_t *t, servo_operations_t *ops)
{
    return producer_consumer_produce(t->ops_pc, ops);
}

void
talking_skull_free_prepared(talking_skull_t *t, servo_operations_t *ops)
{
    servo_operations_destroy(ops);
}

unsigned
talking_skull_play(talking_skull_t *t, unsigned char *data, unsigned n_bytes)
{
    servo_operations_t *ops;

    ops = talking_skull_prepare(t, data, n_bytes);
    ops->free_on_complete = 1;
    return talking_skull_play_prepared(t, ops);
}

void
talking_skull_wait_completion(talking_skull_t *t, unsigned seq)
{
    pthread_mutex_lock(&t->mutex);
    while (t->seq_complete < seq) {
	pthread_cond_wait(&t->cond, &t->mutex);
    }
    pthread_mutex_unlock(&t->mutex);
}

struct talking_skull_actorS {
    wav_t *servo;
    audio_meta_t meta_servo;
    talking_skull_t *talking_skull;
    unsigned char *servo_data;
    size_t n_servo_data;
    servo_operations_t *ops;
};

talking_skull_actor_t *
talking_skull_actor_new_with_n_to_avg(const char *fname, talking_skull_servo_update_t update, void *data, unsigned n_to_avg)
{
    talking_skull_actor_t *a;
    wav_t *servo;

    if ((servo = wav_new(fname)) == NULL) {
	return NULL;
    }

    a = fatal_malloc(sizeof(*a));
    a->servo = servo;
    a->meta_servo = wav_get_meta(a->servo);
    a->servo_data = wav_get_raw_data(a->servo, &a->n_servo_data);
    a->talking_skull = talking_skull_new_with_n_to_avg(&a->meta_servo, n_to_avg, update, data);
    a->ops = talking_skull_prepare(a->talking_skull, a->servo_data, a->n_servo_data);

    return a;
}

talking_skull_actor_t *
talking_skull_actor_new(const char *fname, talking_skull_servo_update_t update, void *data)
{
    return talking_skull_actor_new_with_n_to_avg(fname, update, data, N_TO_AVG_TRACK);
}

talking_skull_actor_t *
talking_skull_actor_new_vsa(const char *fname, talking_skull_servo_update_t update, void *data)
{
    talking_skull_actor_t *a;
    FILE *f;
    unsigned i = 0;
    char buf[128];

    if ((f = fopen(fname, "r")) == NULL) {
	perror(fname);
	return NULL;
    }

    a = fatal_malloc(sizeof(*a));
    a->servo = NULL;
    a->talking_skull = talking_skull_new(NULL, 0, update, data);
    a->ops = servo_operations_new();

    state_start_new_buffer(&a->talking_skull->state);

    while (fgets(buf, sizeof(buf), f) != NULL) {
	int pos = atoi(buf);
	if (i > 0 && pos >= 0) servo_operations_add(a->ops, 1000*1000*(i*.033), pos/254.0 * 100);
	i++;
    }

    return a;
}

talking_skull_actor_t *
talking_skull_actor_new_from_audio(const char *fname, talking_skull_servo_update_t update, void *data)
{
    return talking_skull_actor_new_with_n_to_avg(fname, update, data, N_TO_AVG_GENERATED);
}

void
talking_skull_actor_play(talking_skull_actor_t *a)
{
    talking_skull_play_prepared(a->talking_skull, a->ops);
}
