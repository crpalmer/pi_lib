#ifndef __PRODUCER_CONSUMER_H__
#define __PRODUCER_CONSUMER_H__

typedef struct producer_consumerS producer_consumer_t;

producer_consumer_t *
producer_consumer_new(size_t n_buffers);

void *
producer_consumer_consume(producer_consumer_t *, unsigned *seq);

unsigned
producer_consumer_produce(producer_consumer_t *, void *buffer);

void
producer_consumer_destroy(producer_consumer_t *);

#endif
