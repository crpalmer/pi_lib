#include <stdio.h>
#include "pi.h"
#include "consoles.h"

#include "buffer.h"

BufferFile::BufferFile(const char *fname) : fname(fname) {
    if ((f = media_file_open_read(fname)) == NULL) {
        consoles_fatal_printf("Failed to open %s\n", fname);
    }
}

bool BufferFile::is_eof() {
    return file_is_eof(f);
}

int BufferFile::seek_abs(long pos) {
    return file_seek_abs(f, pos);
}

int BufferFile::seek_rel(long pos) {
    return file_seek_rel(f, pos);
}

BufferFile::~BufferFile() {
    file_close(f);
}

size_t BufferFile::read(void *buf, size_t n) {
    return file_read(f, buf, n);
}

BufferBuffer *BufferFile::get_sub_buffer(size_t n) {
    void *new_data = fatal_malloc(n+1);
    if (read(new_data, n) != n) {
	free(new_data);
	return NULL;
    }
    ((char *) new_data)[n] = '\0';
    return new BufferBuffer(new_data, n);
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

BufferBuffer *BufferBuffer::get_sub_buffer(size_t size) {
    if (at + size > n) return NULL;
    return new BufferBuffer(&((char *) buffer)[at], size);
}

BufferBuffer::~BufferBuffer() {
}
