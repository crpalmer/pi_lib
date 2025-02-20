#ifndef __THREAD_INTERRUPT_NOTIFIER_H__
#define __THREAD_INTERRUPT_NOTIFIER_H__

#include "pi-threads.h"

class ThreadInterruptNotifier : public PiThread, public InputNotifier {
public:
    ThreadInterruptNotifier(const char *name = "interrupt-handler") : PiThread(name) {
	start();
    }
	
    void on_change() {
	resume_from_isr();
    }

    void main(void) {
	while (1) {
	    pause();
	    on_change_safe();
	}
    }

    virtual void on_change_safe() = 0;
};

#endif
