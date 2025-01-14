#ifndef __MEM_H__
#define __MEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <malloc.h>

void mem_set_get_task_name(const char *(*fn)());
void mem_check(void *ptr);

void *fatal_malloc(size_t size);
char *fatal_strdup(const char *str);
void *fatal_realloc(void *, size_t);
void fatal_free(void *ptr);

#ifdef __cplusplus
};
#endif

#endif
