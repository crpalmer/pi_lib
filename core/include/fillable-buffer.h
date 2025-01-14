#ifndef __FILLABLE_BUFFER_H__
#define __FILLABLE_BUFFER_H__

#include "pi-threads.h"

class FillableBuffer : public Buffer {
public:
    FillableBuffer() {
	lock = new PiMutex();
	reader = new PiCond();
	writer = new PiCond();
    }

    ~FillableBuffer() {
	delete writer;
	delete reader;
	delete lock;
    }

    bool is_eof() override { return is_eof_forced; }
    int seek_abs(long pos) override { return -1; }
    int seek_rel(long pos) override { return -1; }
    const char *get_fname() override { return "<play-buffer>"; }
    Buffer *get_sub_buffer(size_t n) override { return NULL; }
    int get_n() override { return 1; }

    void set_is_eof(bool is_eof) {
	lock->lock();
	is_eof_forced = is_eof;
	reader->signal();
	lock->unlock();
    }

    size_t read(void *buffer, size_t n) override {
	lock->lock();
        if (is_eof()) return 0;

	while (to_fill) {
	    reader->wait(lock);
	    if (is_eof()) return 0;
	}

	to_fill = (uint8_t *) buffer;
	to_fill_n = n;
	n_filled = 0;

	while (n_filled == 0) {
	    writer->signal();
	    reader->wait(lock);
	    if (is_eof()) return 0;
	}

	n = n_filled;
	n_filled = 0;

	lock->unlock();

	return n;
    }

    void fill(uint8_t *data, size_t n) {
	lock->lock();

	while (n) {
	    while (! to_fill) writer->wait(lock);
	    size_t this_n = n > to_fill_n ? to_fill_n : n;

	    memcpy(to_fill, data, this_n);
	    data += this_n;
	    to_fill += this_n;
	    n_filled += this_n;

	    n -= this_n;
	    to_fill_n -= this_n;

	    if (to_fill_n == 0) {
		to_fill = NULL;
		reader->signal();
	    }
	}

	lock->unlock();
    }

private:
    bool is_eof_forced = false;
    uint8_t *to_fill = NULL;
    size_t to_fill_n, n_filled = 0;
    PiMutex *lock;
    PiCond *reader, *writer;
};

#endif
