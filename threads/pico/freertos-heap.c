#include <stdio.h>
#include "pi.h"
#include "mem.h"

void *pvPortMalloc(size_t bytes) { return fatal_malloc(bytes); }
void vPortFree(void *ptr) { return fatal_free(ptr); }
