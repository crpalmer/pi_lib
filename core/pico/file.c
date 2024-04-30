#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "file.h"

file_t *file_open_read(const char *fname) { return NULL; }
file_t *file_open_write(const char *fname) { return NULL; }

void file_printf(file_t *file, const char *fmt, ...) {
   va_list va;
   va_start(va, fmt);
   // vfprintf(file, fmt, va);
   va_end(va);
}

int file_scanf(file_t *file, const char *fmt, ...) {
   va_list va;
   va_start(va, fmt);
   //int ret = vfscanf(file, fmt, va);
   va_end(va);

   return 0;
}

size_t file_write(file_t *file, void *data, size_t n_data) { return 0; }
size_t file_read(file_t *file, void *data, size_t n_data) { return 0; }

bool file_seek_abs(file_t *file, unsigned long pos) { return false; }
bool file_seek_rel(file_t *file, long delta) { return false; }

void file_close(file_t *file) { }

bool file_is_eof(file_t *file) { return true; }
