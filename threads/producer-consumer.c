#include "pi.h"
#include "mem.h"
#include "pi-threads.h"
#include "producer-consumer.h"

struct producer_consumerS {
    void **buffers;
    size_t n_buffers;
    size_t n_full;
    size_t next_out, next_in;
    unsigned seq_in, seq_out;
    pi_mutex_t *mutex;
    pi_cond_t *cond;
};

#define BUFFER(pc, i) ((void *) ((pc)->buffers[(i)*(pc)->buffer_size]))
#define BUFFER_IN(pc) BUFFER(pc, (pc)->next_in)
#define BUFFER_OUT(pc) BUFFER(pc, (pc)->next_out)

producer_consumer_t *
producer_consumer_new(size_t n_buffers)
{
    producer_consumer_t *pc = fatal_malloc(sizeof(*pc));
    pc->buffers = fatal_malloc(n_buffers*sizeof(*pc->buffers));
    pc->n_buffers = n_buffers;
    pc->n_full = 0;
    pc->next_in = 0;
    pc->next_out = 0;
    pc->seq_in = 1;
    pc->seq_out = 1;
    pc->mutex = pi_mutex_new();
    pc->cond = pi_cond_new();

    return pc;
}

void *
producer_consumer_consume(producer_consumer_t *pc, unsigned *seq)
{
    void *ret;

    pi_mutex_lock(pc->mutex);
    while (pc->n_full == 0) {
	pi_cond_wait(pc->cond, pc->mutex);
    }

    ret = pc->buffers[pc->next_in];
    pc->next_in = (pc->next_in + 1) % pc->n_buffers;
    pc->n_full--;
    if (seq) *seq = pc->seq_out;
    pc->seq_out++;

    pi_cond_signal(pc->cond);
    pi_mutex_unlock(pc->mutex);

    return ret;
} 

unsigned
producer_consumer_produce(producer_consumer_t *pc, void *buffer)
{
    unsigned seq;

    pi_mutex_lock(pc->mutex);
    while (pc->n_full == pc->n_buffers) {
	pi_cond_wait(pc->cond, &pc->mutex);
    }

    pc->buffers[pc->next_out] = buffer;
    pc->next_out = (pc->next_out + 1) % pc->n_buffers;
    pc->n_full++;

    seq = pc->seq_in++;

    pi_cond_signal(pc->cond);
    pi_mutex_unlock(pc->mutex);

    return seq;
}

void
producer_consumer_destroy(producer_consumer_t *pc)
{
    pi_mutex_destroy(pc->mutex);
    pi_cond_destroy(pc->cond);
    fatal_free(pc->buffers);
    fatal_free(pc);
}
