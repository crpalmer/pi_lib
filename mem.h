#ifndef __MEM_H__
#define __MEM_H__

#include <malloc.h>

void *fatal_malloc(size_t size);
char *fatal_strdup(const char *str);
void *fatal_realloc(void *, size_t);

#endif
