#ifndef __FILE_H__
#define __FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/stat.h>
#include "file-platform.h"

file_t *media_file_open_read(const char *fname);

off_t file_size(const char *fname);

file_t *file_open(const char *fname, const char *mode);

size_t file_read(file_t *file, void *data, size_t size);
size_t file_write(file_t *file, void *data, size_t size);

bool file_seek_abs(file_t *file, unsigned long pos);
bool file_seek_rel(file_t *file, long delta);

void file_printf(file_t *file, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
int file_scanf(file_t *file, const char *fmt, ...) __attribute__ ((format (scanf, 2, 3)));

void file_close(file_t *);

bool file_is_eof(file_t *);

#ifdef __cplusplus
};
#endif

#endif
