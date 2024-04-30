#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "consoles.h"
#include "mem.h"
#include "time-utils.h"
#include "talking-skull.h"

TalkingSkullFileOps::TalkingSkullFileOps(const char *fname) {
    if ((f = file_open_read(fname)) == NULL) {
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

/* -------------------------------------------------------------- */

TalkingSkull::TalkingSkull(const char *thread_name) : PiThread(thread_name) {
    wait_lock = new PiMutex();
    wait_cond = new PiCond();
    n_pos = 0;
    a_pos = 16;
    pos = (double *) fatal_malloc(a_pos * sizeof(*pos));
    usec_per_i = 0;

    start();
}

void TalkingSkull::ops(TalkingSkullOps *ops) {
    wait_lock->lock();

    usec_per_i = ops->get_usec_per_i();
    n_pos = 0;

    double next_pos;
    while (ops->next(&next_pos)) {
	if (n_pos >= a_pos) {
	    a_pos *= 2;
	    pos = (double *) fatal_realloc(pos, a_pos * sizeof(*pos));
	}
	pos[n_pos++] = next_pos;
    }

    wait_lock->unlock();
}

TalkingSkull::~TalkingSkull() {
    delete wait_lock;
    delete wait_cond;
    free(pos);
}

void TalkingSkull::play() {
    wait_lock->lock();
    wait_cond->signal();
    wait_lock->unlock();
}

void
TalkingSkull::main() {
    wait_lock->lock();
    while (1) {
	wait_cond->wait(wait_lock);
	
	struct timespec next;
	unsigned cur = 0;

	nano_gettime(&next);

	while (cur < n_pos) {
	    struct timespec now;

	    nano_gettime(&now);
	    nano_add_usec(&next, usec_per_i);
	    if (nano_later_than(&next, &now)) {
		nano_sleep_until(&next);
		update_pos(pos[cur]);
	    }
	    cur++;
	}
    }
}

/* -------------------------------------------------------------- */

int
talking_skull_ops_to_filename(const char *fname, TalkingSkullOps *ops) {
    file_t *f;

    if ((f = file_open_write(fname)) == NULL) return -1;
    int ret = talking_skull_ops_to_file(f, ops);
    file_close(f);

    return ret;
}

int
talking_skull_ops_to_file(file_t *f, TalkingSkullOps *ops) {
    file_printf(f, "%d", ops->get_usec_per_i());
    double pos;
    for (int i = 0; ops->next(&pos); i++) {
	file_printf(f, "%c%g", i % 10 == 0 ? '\n' : ' ', pos);
    }
    return 0;
}
