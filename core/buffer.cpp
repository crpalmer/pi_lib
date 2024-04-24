#include <stdio.h>
#include "consoles.h"
#include "file.h"

#include "buffer.h"

BufferFile::BufferFile(const char *fname) : fname(fname) {
    if ((f = media_fopen_read(fname)) == NULL) {
        consoles_fatal_printf("Failed to open %s\n", fname);
    }
}

bool BufferFile::is_eof() {
    return feof(f);
}

int BufferFile::seek_abs(long pos) {
    return fseek(f, pos, SEEK_SET);
}

int BufferFile::seek_rel(long pos) {
    return fseek(f, pos, SEEK_CUR);
}

BufferFile::~BufferFile() {
    fclose(f);
}

int BufferFile::read(void *buf, size_t n) {
    return fread(buf, 1, n, f);
}

int BufferBuffer::read(void *user_buf, size_t buf_size) {
    size_t to_read;

    if (at >= n) return 0;
    if (at + buf_size <= n) to_read = buf_size;
    else to_read = n - at;

//printf("read: %6d @ %6d (total %d)\n", to_read, at, n);
    memcpy(user_buf, &((uint8_t *) buffer)[at], to_read);
    at += to_read;

    return to_read;
}

int BufferBuffer::seek_abs(long pos) {
    if (pos < 0) at = 0;
    else if (pos > n) at = n;
    else at = pos;
    return 0;
}

int BufferBuffer::seek_rel(long pos) {
    if (-pos > (long) at) at = 0;
    else if (pos + at > n) at = n;
    else at += pos;
    return 0;
}
