#include <stdio.h>
#include "pi.h"
#include "consoles.h"

#include "buffer.h"

BufferFile::BufferFile(const char *fname, long start, long max_bytes, file_t *f) : fname(fname), at(0), start(start), max_bytes(max_bytes), f(f) {
    if (! f && (f = media_file_open_read(fname)) == NULL) {
        consoles_fatal_printf("Failed to open %s\n", fname);
    }
}

bool BufferFile::is_eof() {
    return (max_bytes >= 0 && at >= max_bytes) || file_is_eof(f);
}

int BufferFile::seek_abs(long pos) {
    at = pos;
    if (at < 0) at = 0;
    if (max_bytes > 0 && at >= max_bytes) at = max_bytes - 1;
    return file_seek_abs(f, start+at);
}

int BufferFile::seek_rel(long pos) {
    at += pos;
    return file_seek_abs(f, at);
}

BufferFile::~BufferFile() {
    file_close(f);
}

size_t BufferFile::read(void *buf, size_t n) {
    if (max_bytes >= 0 && at + (long) n > max_bytes) n = max_bytes - at;
    size_t did = file_read(f, buf, n);
    if (did > 0) at += did;
    return did;
}

Buffer *BufferFile::get_sub_buffer(size_t n) {
    return new BufferFile(fname, at, n);
}

size_t BufferBuffer::read(void *user_buf, size_t buf_size) {
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
    else if ((size_t) pos > n) at = n;
    else at = pos;
    return 0;
}

int BufferBuffer::seek_rel(long pos) {
    if (-pos > (long) at) at = 0;
    else if (pos + at > n) at = n;
    else at += pos;
    return 0;
}

Buffer *BufferBuffer::get_sub_buffer(size_t size) {
    if (at + size > n) return NULL;
    return new BufferBuffer(&((char *) buffer)[at], size);
}

BufferBuffer::~BufferBuffer() {
}

BufferFile *buffer_file_open(const char *fname) {
    file_t *f;
    if ((f = media_file_open_read(fname)) == NULL) {
        consoles_printf("Failed to open %s\n", fname);
	return NULL;
    }
    return new BufferFile(fname, 0, -1, f);
}
