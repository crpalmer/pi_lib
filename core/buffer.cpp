#include <stdio.h>
#include "pi.h"
#include "consoles.h"

#include "buffer.h"

FileBuffer::FileBuffer(const char *fname, long start, long max_bytes, file_t *user_f) : fname(fatal_strdup(fname)), at(0), start(start), max_bytes(max_bytes) {
    if (user_f) {
	f = user_f;
    } else if ((f = media_file_open_read(fname)) == NULL) {
        consoles_fatal_printf("Failed to open %s\n", fname);
    }
}

bool FileBuffer::is_eof() {
    return (max_bytes >= 0 && at >= max_bytes) || file_is_eof(f);
}

int FileBuffer::seek_abs(long pos) {
    at = pos;
    if (at < 0) at = 0;
    if (max_bytes > 0 && at >= max_bytes) at = max_bytes - 1;
    return file_seek_abs(f, start+at);
}

int FileBuffer::seek_rel(long pos) {
    at += pos;
    return file_seek_abs(f, at);
}

FileBuffer::~FileBuffer() {
    file_close(f);
    fatal_free(fname);
}

size_t FileBuffer::read(void *buf, size_t n) {
    if (max_bytes >= 0 && at + (long) n > max_bytes) n = max_bytes - at;
    size_t did = file_read(f, buf, n);
    if (did > 0) at += did;
    return did;
}

Buffer *FileBuffer::get_sub_buffer(size_t n) {
    return new FileBuffer(fname, at, n);
}

size_t MemoryBuffer::read(void *user_buf, size_t buf_size) {
    size_t to_read;

    if (at >= n) return 0;
    if (at + buf_size <= n) to_read = buf_size;
    else to_read = n - at;

//printf("read: %6d @ %6d (total %d)\n", to_read, at, n);
    memcpy(user_buf, &((uint8_t *) buffer)[at], to_read);
    at += to_read;

    return to_read;
}

int MemoryBuffer::seek_abs(long pos) {
    if (pos < 0) at = 0;
    else if ((size_t) pos > n) at = n;
    else at = pos;
    return 0;
}

int MemoryBuffer::seek_rel(long pos) {
    if (-pos > (long) at) at = 0;
    else if (pos + at > n) at = n;
    else at += pos;
    return 0;
}

Buffer *MemoryBuffer::get_sub_buffer(size_t size) {
    if (at + size > n) return NULL;
    return new MemoryBuffer(&((char *) buffer)[at], size);
}

MemoryBuffer::~MemoryBuffer() {
}

FileBuffer *file_buffer_open(std::string fname) {
    file_t *f;
    if ((f = media_file_open_read(fname.c_str())) == NULL) {
        consoles_printf("Failed to open %s\n", fname.c_str());
	return NULL;
    }
    return new FileBuffer(fname.c_str(), 0, -1, f);
}
