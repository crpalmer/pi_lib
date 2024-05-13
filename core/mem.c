#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pi.h"
#include "consoles.h"
#include "mem.h"

static const bool memory_check = false;
static const bool memory_trace = false;
static const bool memory_overwrite_new = false;

#define PAD_SIZE 8

const char *freed_str = "*FREED*";	/* note: 7 chars plus \0 */

static const char *(*get_task_name_fn)() = NULL;

static const char *get_task_name() {
    if (get_task_name_fn) return (*get_task_name_fn)();
    else return "<unknown>";
}

void mem_set_get_task_name(const char *(*fn)()) {
    get_task_name_fn = fn;
}

typedef struct {
    const char *who;
    size_t size;
    char pad[PAD_SIZE];
} mem_header_t;

static int extra_bytes() {
    if (memory_check) return sizeof(mem_header_t) + PAD_SIZE;
    else return 0;
}

static void overwrite_new(void *ptr, size_t size) {
    if (memory_overwrite_new) {
	uint8_t *p = (uint8_t *) ptr;
	for (size_t i = 0; i < size; i++) p[i] = (i+1) % 0xff;
    }
}

static void add_padding(void *ptr) {
    if (! memory_check) return;
    uint8_t *p = (uint8_t *) ptr;
    for (int i = 0; i < PAD_SIZE; i++) p[i] = i % 0xff;
}

static void *
record_memory(void *p, size_t size) {
    if (memory_check) {
	mem_header_t *hdr = (mem_header_t *) p;
	p += sizeof(*hdr);
	hdr->who = get_task_name();
	hdr->size = size;
	add_padding(hdr->pad);
	add_padding(p + size);
    }
    return p;
}

static void check_padding(void *ptr, void *user_ptr, mem_header_t *hdr, bool is_free) {
    if (! memory_check) return;
    if (strncmp(ptr, freed_str, strlen(freed_str)) == 0) {
	fprintf(stderr, "%s: double free of %p\n", hdr->who, user_ptr);
	assert(0);
    }
    uint8_t *p = (uint8_t *) ptr;
    for (int i = 0; i < PAD_SIZE && p; i++) {
	if (p[i] != i % 0xff) {
	    fprintf(stderr, "%s: padding check failure: %p %d @ %d = %p\n", hdr->who, user_ptr, (int) hdr->size, i, &p[i]);
	    assert(0);
	}
    }
    if (is_free) strcpy(ptr, freed_str);
}

static void *
check_memory(void *ptr, bool is_free) {
    if (memory_check) {
	mem_header_t *hdr = (mem_header_t *) (ptr - sizeof(*hdr));
	check_padding(hdr->pad, ptr, hdr, is_free);
	check_padding(ptr + hdr->size, ptr, hdr, is_free);
	ptr = hdr;
    }
    return ptr;
}

static int
get_size_if_available(void *ptr) {
    if (memory_check) {
	mem_header_t *hdr = (mem_header_t *) (ptr - sizeof(*hdr));
	return hdr->size;
    } else {
	return -1;
    }
}

void
mem_check(void *ptr) {
    check_memory(ptr, false);
}

void *
fatal_malloc(size_t size)
{
    void *ptr;

    if ((ptr = malloc(size + extra_bytes())) == NULL) {
	consoles_fatal_printf("Failed to allocated %ld bytes.\n", (long) size);
    }
    ptr = record_memory(ptr, size);
    overwrite_new(ptr, size);
    if (memory_trace) printf("%s: %p: malloc %d\n", get_task_name(), ptr, (int) size);
    return ptr;
}

char *
fatal_strdup(const char *str)
{
    char *ptr = fatal_malloc(strlen(str)+1);
    strcpy(ptr, str);
    if (memory_trace) printf("%s: %p: strdup %d\n", get_task_name(), ptr, (int) strlen(ptr));
    return ptr;
}

void *
fatal_realloc(void *ptr, size_t size)
{
    if (memory_trace) printf("%s: %p: realloc %d", get_task_name(), ptr, (int) size);

    ptr = check_memory(ptr, true);

    if ((ptr = realloc(ptr, size + extra_bytes())) == NULL) {
	consoles_fatal_printf("Failed to realloc %ld bytes.\n", (long) size);
    }

    ptr = record_memory(ptr, size);
    overwrite_new(ptr, size);

    if (memory_trace) printf(" => %p\n", ptr);
    return ptr;
}

void
fatal_free(void *ptr) {
    if (memory_trace) printf("%s: %p: free %d\n", get_task_name(), ptr, get_size_if_available(ptr));
    ptr = check_memory(ptr, true);
    free(ptr);
}

