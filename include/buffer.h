#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <string>
#include <stdint.h>
#include <string.h>

class BufferBuffer;

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
};

class BufferFile : public Buffer {
public:
    BufferFile(const char *fname, long start = 0, long max_bytes = -1, file_t *f = NULL);
    ~BufferFile() override;
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

class BufferBuffer : public Buffer {
public:
    BufferBuffer(const void *buffer, size_t n) : buffer(buffer), n(n), at(0) {}
    BufferBuffer(const char *text) : buffer(text), n(strlen(text)), at(0) {}
    ~BufferBuffer() override;
    bool is_eof() override { return at == n; }
    size_t read(void *buf, size_t buf_size) override;
    int seek_abs(long pos) override;
    int seek_rel(long pos) override;
    const char *get_fname() override { return "<buffer>"; }
    Buffer *get_sub_buffer(size_t n) override;

    const void *get_data() { return buffer;}
    int get_n() override { return n; }

private:
    const void *buffer;
    size_t n, at;
};

BufferFile *buffer_file_open(std::string fname);

#endif
