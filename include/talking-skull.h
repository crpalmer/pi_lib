#ifndef __TALKING_SKULL_H__
#define __TALKING_SKULL_H__

#include <stdio.h>
#include "pi-threads.h"

class TalkingSkullOps {
public:
    virtual ~TalkingSkullOps() { }
    virtual int get_usec_per_i() = 0;
    virtual bool next(double *pos) = 0;
    virtual bool reset() = 0;
    virtual int get_n_ops() = 0;
};

class TalkingSkull : PiThread {
public:
    TalkingSkull(const char *thread_name = "talking-skull", int bytes_per_op = 2);
    ~TalkingSkull();

    void set_ops(TalkingSkullOps *ops);
    void play();
    void wait_done();

    void main() override;

    virtual void update_pos(double pos) = 0;

protected:
    PiMutex *wait_lock;
    PiCond  *wait_cond;

    int bytes_per_op;
    uint8_t *ops;
    int n_ops;

    uint32_t op_bits;
    unsigned usec_per_i;
};

#endif
