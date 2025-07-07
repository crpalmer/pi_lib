#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <string>
#include <stdint.h>
#include <string.h>
#include "mem.h"

class MemoryBuffer;

class Buffer {
public:
    virtual ~Buffer() {}
    virtual bool is_eof() = 0;
    virtual size_t read(void *buf, size_t n) = 0;
    virtual int seek_abs(long pos) = 0;
    virtual int seek_rel(long pos) = 0;
    virtual const char *get_fname() = 0;
    virtual uint8_t next() {
	uint8_t byte;
	read(&byte, 1);
	return byte;
    }
    virtual Buffer *get_sub_buffer(size_t n) = 0;
    virtual int get_n() = 0;
    virtual const void *get_raw_data() { return NULL; }
};

class FileBuffer : public Buffer {
public:
    FileBuffer(const char *fname, long start = 0, long max_bytes = -1, file_t *f = NULL);
    ~FileBuffer() override;
    bool is_eof() override;
    size_t read(void *buf, size_t n) override;
    int seek_abs(long pos) override;
    int seek_rel(long pos) override;
    const char *get_fname() override { return fname; }
    Buffer *get_sub_buffer(size_t n) override;

    int get_n() override {
	if (max_bytes >= 0) return max_bytes;
	else return (int) file_size(fname);
    }

private:
    char *fname;
    long at, start, max_bytes;
    file_t *f;
};

class MemoryBuffer : public Buffer {
public:
    MemoryBuffer(const void *buffer, size_t n) : buffer(buffer), n(n), at(0) {}
    MemoryBuffer(const char *str) : buffer(str), n(strlen(str)), at(0) {}
    ~MemoryBuffer() override;
    bool is_eof() override { return at == n; }
    size_t read(void *buf, size_t buf_size) override;
    int seek_abs(long pos) override;
    int seek_rel(long pos) override;
    const char *get_fname() override { return "<buffer>"; }
    Buffer *get_sub_buffer(size_t n) override;

    const void *get_data() { return buffer;}
    int get_n() override { return n; }

    const void *get_raw_data() override { return buffer; }

protected:
    const void *buffer;
    size_t n, at;
};

class StrdupBuffer : public MemoryBuffer {
public:
    StrdupBuffer(std::string string) : MemoryBuffer(fatal_strdup(string.c_str()), string.length()) { }
    ~StrdupBuffer() override { fatal_free((void *) buffer); }
};

FileBuffer *file_buffer_open(std::string fname);

#endif
