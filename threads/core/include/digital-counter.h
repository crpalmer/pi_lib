#ifndef __DIGITAL_COUNTER_H__
#define __DIGITAL_COUNTER_H__

/* wiring:

   You need the wb to have a high side driver for the selected bank.
   You need to have it set to 5V power if you are using a 5V relay.
   You then connect inc/dec to the indicated pins.
   You then connect the relay with
      - gnd and signal attached to gnd
      - +V attached to the indicated pin
 */

#include "io.h"
#include "pi-threads.h"

class DigitalCounter : public PiThread {
public:
    DigitalCounter(Output *inc, Output *dec, Output *reset, const char *name = "d-counter");
    ~DigitalCounter();

    void main(void) override;

    void add(int delta);
    void set(unsigned value);
    void set_pause(int pause = -1, int reset_pause = -1, int post_reset_pause = -1);

private:
    Output *inc, *dec, *reset;
    PiMutex *lock;
    PiCond *cond;
    int target, actual;
    bool stop;
    unsigned pause, reset_pause, post_reset_pause;
};

#endif
