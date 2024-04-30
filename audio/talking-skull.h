#ifndef __TALKING_SKULL_H__
#define __TALKING_SKULL_H__

#include <stdio.h>
#include "pi-threads.h"

class TalkingSkullOps {
public:
    virtual ~TalkingSkullOps() { }
    virtual int get_usec_per_i() = 0;
    virtual bool next(double *pos) = 0;
};

class TalkingSkullFileOps : public TalkingSkullOps {
public:
    TalkingSkullFileOps(const char *fname);
    ~TalkingSkullFileOps() override;

    int get_usec_per_i() override { return usec_per_i; }
    bool next(double *pos) override;

private:
    file_t *f;
    int usec_per_i;
};

int talking_skull_ops_to_filename(const char *fname, TalkingSkullOps *ops);
int talking_skull_ops_to_file(file_t *f, TalkingSkullOps *ops);

class TalkingSkull : PiThread {
public:
    TalkingSkull(const char *thread_name = "talking-skull");
    ~TalkingSkull();

    void ops(TalkingSkullOps *ops);
    void play();

    void main() override;

    virtual void update_pos(double pos) = 0;

protected:
    PiMutex *wait_lock;
    PiCond  *wait_cond;
    int usec_per_i;
    double *pos;
    size_t n_pos, a_pos;
};

#endif
