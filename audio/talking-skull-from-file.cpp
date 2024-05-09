#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "consoles.h"
#include "talking-skull-from-file.h"

TalkingSkullFileOps::TalkingSkullFileOps(const char *fname) {
    if ((f = file_open(fname, "rb")) == NULL) {
	consoles_fatal_printf("Could not open: %s\n", fname);
    }
    file_scanf(f, "%d", &usec_per_i);
}

TalkingSkullFileOps::~TalkingSkullFileOps() {
    file_close(f);
}

bool TalkingSkullFileOps::next(double *pos) {
    return file_scanf(f, "%lg", pos) == 1;
}

bool TalkingSkullFileOps::reset() {
    return file_seek_abs(f, 0);
}

int TalkingSkullFileOps::get_n_ops() {
    consoles_fatal_printf("I have to implement this!\n");
    return 0;
}
