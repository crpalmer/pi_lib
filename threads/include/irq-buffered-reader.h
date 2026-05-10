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
	    read_all_locked();
	    lock->unlock();
	}
    }

    virtual unsigned char getc() {
	lock->lock();

	while (n == 0) cond->wait(lock);

	unsigned char c = buffer[low];
	low = (low+1) % buffer_n;
	n--;

	/* Call read_all_locked() now in case we previously
	 * were in the middle of performing a read_all_locked()
	 * and ran out of buffer space.  In that case, we'll
	 * end up with data pending but no irq to let us process it.
	 */
	if (n == buffer_n-1) read_all_locked();

	lock->unlock();

	return c;
    }

protected:
    virtual bool read_char_if_available(unsigned char *chr) = 0;

private:
    void read_all_locked() {
	bool should_broadcast = false;

	while (1) {
	    unsigned char c;

	    if (n >= buffer_n) return;
	    if (! read_char_if_available(&c)) return;
	    buffer[high] = c;
	    high = (high+1) % buffer_n;
	    n++;
	    should_broadcast = true;
	}
	if (should_broadcast) cond->broadcast();
    }

    PiMutex *lock;
    PiCond *cond;

    static const int buffer_n = 1024;
    unsigned char buffer[buffer_n];
    int n = 0;
    int low = 0, high = 0;
};

#endif
