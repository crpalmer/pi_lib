#ifndef __TALKING_SKULL_H__
#define __TALKING_SKULL_H__

#include <stdio.h>
#include "pi-threads.h"
#include "talking-skull.h"

class TalkingSkullFileOps : public TalkingSkullOps {
public:
    TalkingSkullFileOps(const char *fname);
    ~TalkingSkullFileOps() override;

    int get_usec_per_i() override { return usec_per_i; }
    bool next(double *pos) override;
    bool reset() override;
    int get_n_ops() override;

private:
    file_t *f;
    int usec_per_i;
};

extern "C" int talking_skull_ops_to_filename(const char *fname, TalkingSkullOps *ops);
extern "C" int talking_skull_ops_to_file(file_t *f, TalkingSkullOps *ops);

#endif
