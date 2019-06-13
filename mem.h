#ifndef __MEM_H__
#define __MEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <malloc.h>

void *fatal_malloc(size_t size);
char *fatal_strdup(const char *str);
void *fatal_realloc(void *, size_t);

#ifdef __cplusplus
};
#endif

#endif
