#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include "mem.h"
#include "producer-consumer.h"
#include "time-utils.h"
#include "talking-skull.h"

#define PRINT_SERVO     0
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

typedef struct {
    size_t n, a;
    servo_data_t *servo;
} servo_operations_t;

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
};

static servo_operations_t *
servo_operations_new(void)
{
    servo_operations_t *ops = fatal_malloc(sizeof(*ops));

    ops->n = 0;
    ops->a = 1024;
    ops->servo = fatal_malloc(ops->a * sizeof(*ops->servo));

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
	next = start;
	nano_add_usec(&next, ops->servo[cur].usec);
	nano_sleep_until(&next);
	fn(fn_data, ops->servo[cur].pos);
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
state_start_new_buffer(state_t *s)
{
    s->last_usec = s->last_printed_usec = 0;
    s->i = 0;
}

static void
state_update(state_t *s, servo_operations_t *ops, unsigned val)
{
    s->i++;

    if (val > s->mx) s->mx = val;

    if (++s->n_samples % s->n_per_servo == 0) {
	s->sum += s->mx;
	s->mx = 0;
    }

    if (s->n_samples % (s->n_per_servo * s->n_to_avg) == 0) {
	unsigned usec;
	unsigned this_usec = s->i * s->i_to_usec;
	double pos;

	usec = (this_usec - s->last_usec) / 2 + s->last_usec;
	pos = ((double) s->sum) / s->n_to_avg / s->max_possible * 100;
	s->last_usec = this_usec;

	servo_operations_add(ops, usec, pos);

	if (PRINT_SERVO) {
	    if (usec - s->last_printed_usec > 20*1000) {
		unsigned this_val = pos / 2 + 0.5;
		printf("%9.6f:%*c%*c\n", usec / (1000.0*1000.0), this_val+1, '*', 50 - this_val, '|');
		s->last_printed_usec = usec;
	    }
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
	servo_operations_destroy(ops);

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
    talking_skull_t *t;

    t = fatal_malloc(sizeof(*t));
    t->m = *m;
    t->ops_pc = producer_consumer_new(1);
    t->seq_complete = 0;
    pthread_mutex_init(&t->mutex, NULL);
    pthread_cond_init(&t->cond, NULL);

    if (is_track) {
	t->m.num_channels = 1;
    }

    state_init(&t->state, &t->m, is_track);
    t->fn = fn;
    t->fn_data = fn_data;

    pthread_create(&t->thread, NULL, update_main, t);

    return t;
}

unsigned
talking_skull_play(talking_skull_t *t, unsigned char *data, unsigned n_bytes)
{
    size_t i;
    servo_operations_t *ops;

    ops = servo_operations_new();

    state_start_new_buffer(&t->state);

    for (i = 0; i < n_bytes; ) {
	unsigned val = decode_value(&t->m, data, &i, t->state.max_possible);
	state_update(&t->state, ops, val);
    }

    return producer_consumer_produce(t->ops_pc, ops);
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
