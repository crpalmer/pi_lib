#include "pi.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "consoles.h"
#include "mem.h"

#include "pi-threads.h"
#include "pi-threads-queue.h"

struct pi_threads_queue_stateS {
    QueueHandle_t queue;
};

PiThreadsQueue::PiThreadsQueue(int data_size, int max) {
    state = (pi_threads_queue_state_t *) fatal_malloc(sizeof(*state));
    state->queue = xQueueCreate(max, data_size);
    if (! state->queue) consoles_fatal_printf("Failed to create freertos queue!\n");
}

PiThreadsQueue::~PiThreadsQueue() {
    vQueueDelete(state->queue);
    free(state);
}

bool PiThreadsQueue::enqueue(void *data) {
    return xQueueSendToBack(state->queue, data, portMAX_DELAY) == pdTRUE;
}

bool PiThreadsQueue::pop(void *data) {
    return xQueueReceive(state->queue, data, portMAX_DELAY) == pdTRUE;
}
