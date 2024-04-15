#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"

void *
fatal_malloc(size_t size)
{
    void *p;

    if ((p = malloc(size)) == NULL) {
	fprintf(stderr, "Failed to allocated %ld bytes.\n", (long) size);
	exit(1);
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
	fprintf(stderr, "Failed to realloc %ld bytes.\n", (long) size);
	exit(1);
    }

    return ptr;
}
