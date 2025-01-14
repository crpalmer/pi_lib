#ifndef __PI_THREADS_QUEUE_H__
#define __PI_THREADS_QUEUE_H__

typedef struct pi_threads_queue_stateS pi_threads_queue_state_t;

class PiThreadsQueue {
public:
    PiThreadsQueue(int data_size, int max = 100);
    ~PiThreadsQueue();

    bool enqueue(void *data);
    bool pop(void *datea);

private:
    pi_threads_queue_state_t *state;
};

#endif
