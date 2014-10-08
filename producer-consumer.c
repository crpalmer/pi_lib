#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "mem.h"
#include "producer-consumer.h"

struct producer_consumerS {
    void **buffers;
    size_t n_buffers;
    size_t n_full;
    size_t next_out, next_in;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
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
    pthread_mutex_init(&pc->mutex, NULL);
    pthread_cond_init(&pc->cond, NULL);

    return pc;
}

void *
producer_consumer_consume(producer_consumer_t *pc)
{
    void *ret;

    pthread_mutex_lock(&pc->mutex);
    while (pc->n_full == 0) {
	pthread_cond_wait(&pc->cond, &pc->mutex);
    }

    ret = pc->buffers[pc->next_in];
    pc->next_in = (pc->next_in + 1) % pc->n_buffers;
    pc->n_full--;

    pthread_cond_signal(&pc->cond);
    pthread_mutex_unlock(&pc->mutex);

    return ret;
} 

void
producer_consumer_produce(producer_consumer_t *pc, void *buffer)
{
    pthread_mutex_lock(&pc->mutex);
    while (pc->n_full == pc->n_buffers) {
	pthread_cond_wait(&pc->cond, &pc->mutex);
    }

    pc->buffers[pc->next_out] = buffer;
    pc->next_out = (pc->next_out + 1) % pc->n_buffers;
    pc->n_full++;

    pthread_cond_signal(&pc->cond);
    pthread_mutex_unlock(&pc->mutex);
}

void
producer_consumer_destroy(producer_consumer_t *pc)
{
    pthread_mutex_destroy(&pc->mutex);
    pthread_cond_destroy(&pc->cond);
    free(pc->buffers);
    free(pc);
}
