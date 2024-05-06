#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "consoles.h"
#include "mem.h"

void *
fatal_malloc(size_t size)
{
    void *p;

    if ((p = malloc(size)) == NULL) {
	consoles_fatal_printf("Failed to allocated %ld bytes.\n", (long) size);
    }
    return p;
}

char *
fatal_strdup(const char *str)
{
    char *p = fatal_malloc(strlen(str)+1);
    strcpy(p, str);
    return p;
}

void *
fatal_realloc(void *ptr, size_t size)
{
    if ((ptr = realloc(ptr, size)) == NULL) {
	consoles_fatal_printf("Failed to realloc %ld bytes.\n", (long) size);
    }

    return ptr;
}

void
fatal_free(void *ptr) {
    free(ptr);
}

