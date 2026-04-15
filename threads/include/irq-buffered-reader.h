#ifndef __IRQ_BUFFERED_READER_H__
#define __IRQ_BUFFERED_READER_H__

#include "pi-threads.h"

class IRQBufferedReader : public PiThread {
public:
    IRQBufferedReader() : PiThread("stdin-buffer") {
	lock = new PiMutex();
	cond = new PiCond();
	start();
    }

    void main(void) {
	while (1) {
	    pause();
	    lock->lock();
	    read_all();
	    lock->unlock();
	    cond->broadcast();
	}
    }

    unsigned char getchar() {
	lock->lock();
	while (n == 0) cond->wait(lock);

	unsigned char c = buffer[low];
	low = (low+1) % buffer_n;
	n--;

	lock->unlock();

	return c;
    }

protected:
    virtual bool read_char_if_available(int *chr) = 0;

private:
    void read_all() {
	while (1) {
	    int c;

	    if (n >= buffer_n) return;
	    if (! read_char_if_available(&c)) return;
	    buffer[high] = c;
	    high = (high+1) % buffer_n;
	    n++;
	}
    }

private:
    PiMutex *lock;
    PiCond *cond;

    static const int buffer_n = 1024;
    unsigned char buffer[buffer_n];
    int n = 0;
    int low = 0, high = 0;
};

#endif
