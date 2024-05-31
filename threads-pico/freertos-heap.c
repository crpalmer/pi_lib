#include <stdio.h>
#include <malloc.h>
#include "pi.h"
#include "mem.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "freertos-heap.h"

#if 1
static SemaphoreHandle_t malloc_lock;

void malloc_lock_init() {
    malloc_lock = xSemaphoreCreateRecursiveMutex();
}

void __malloc_lock(struct _reent *r) {
    if (malloc_lock) xSemaphoreTakeRecursive(malloc_lock, portMAX_DELAY);
}

void __malloc_unlock(struct _reent *r) {
    if (malloc_lock) xSemaphoreGiveRecursive(malloc_lock);
}

#else

void malloc_lock_init() {
}

#endif

void *pvPortMalloc(size_t bytes) {
    return fatal_malloc(bytes);
}

void vPortFree(void *ptr) {
    fatal_free(ptr);
}
