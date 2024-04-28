#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <stdint.h>

class BufferBuffer;

class Buffer {
public:
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
    virtual BufferBuffer *get_sub_buffer(size_t n) = 0;
};

class BufferFile : public Buffer {
public:
    BufferFile(const char *fname);
    ~BufferFile();
    bool is_eof() override;
    size_t read(void *buf, size_t n) override;
    int seek_abs(long pos) override;
    int seek_rel(long pos) override;
    const char *get_fname() override { return fname; }
    BufferBuffer *get_sub_buffer(size_t n) override;

private:
    const char *fname;
    FILE *f;
};

class BufferBuffer : public Buffer {
public:
    BufferBuffer(const void *buffer, size_t n) : buffer(buffer), n(n), at(0) {}
    bool is_eof() override { return at == n; }
    size_t read(void *buf, size_t buf_size) override;
    int seek_abs(long pos) override;
    int seek_rel(long pos) override;
    const char *get_fname() override { return "<buffer>"; }
    BufferBuffer *get_sub_buffer(size_t n) override;

    const void *get_data() { return buffer;}
    size_t get_n_data() { return n; }

private:
    const void *buffer;
    size_t n, at;
};

#endif
