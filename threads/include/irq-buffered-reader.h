#ifndef __IRQ_BUFFERED_READER_H__
#define __IRQ_BUFFERED_READER_H__

#include "pi-threads.h"

class IRQBufferedReader : public PiThread {
public:
    IRQBufferedReader(const char *name = "irq-buffered-reader") : PiThread(name) {
	lock = new PiMutex();
	cond = new PiCond();
	start();
    }

    void main(void) {
	while (1) {
	    lock->lock();
	    read_all_locked();
	    lock->unlock();
	    pause();
	}
    }

    bool is_empty() {
	lock->lock();
	bool result = (n == 0);
	lock->unlock();

	return result;
    }

    unsigned char getc() {
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
	read_all_locked();

	lock->unlock();
	return c;
    }

    void on_irq() {
	resume_from_isr();
    }

protected:
    virtual bool read_char_if_available(unsigned char *chr) = 0;

private:
    void read_all_locked() {
	unsigned char c;
	while (n < buffer_n && read_char_if_available(&c)) {
	    buffer[high] = c;
	    high = (high+1) % buffer_n;
	    n++;
	}
	if (n > 0) cond->broadcast();
    }

    PiMutex *lock;
    PiCond *cond;

    static const int buffer_n = 1024;
    unsigned char buffer[buffer_n];
    int n = 0;
    int low = 0, high = 0;
};

#endif
