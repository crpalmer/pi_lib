#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "consoles.h"
#include "mem.h"
#include "time-utils.h"
#include "talking-skull.h"

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

/* -------------------------------------------------------------- */

TalkingSkull::TalkingSkull(const char *thread_name, int bytes_per_op) : PiThread(thread_name), bytes_per_op(bytes_per_op) {
    wait_lock = new PiMutex();
    wait_cond = new PiCond();

    a_ops = 32;
    n_ops = 0;
    ops = (uint8_t *) fatal_malloc(a_ops * bytes_per_op);

    op_bits = (1 << (8*bytes_per_op)) - 1;

    start();
}

TalkingSkull::~TalkingSkull() {
    delete wait_lock;
    delete wait_cond;
    fatal_free(ops);
}

void TalkingSkull::set_ops(TalkingSkullOps *skull_ops) {
    wait_lock->lock();

    usec_per_i = skull_ops->get_usec_per_i();
    n_ops = 0;

    double pos;
    while (skull_ops->next(&pos)) {
	if (n_ops >= a_ops) {
	    if (a_ops < 1024) a_ops *= 2;
	    else a_ops += 512;
	    ops = (uint8_t *) realloc(ops, a_ops * bytes_per_op);
	}
	uint32_t encoded = pos / 100 * op_bits;
	for (int i = 0; i < bytes_per_op; i++) ops[n_ops * bytes_per_op + i] = (encoded >> (8*i)) & 0xff;
 	n_ops++;
    }

    wait_lock->unlock();
}

void TalkingSkull::play() {
    wait_lock->lock();
    wait_cond->signal();
    wait_lock->unlock();
}

void TalkingSkull::wait_done() {
    wait_lock->lock();
    wait_lock->unlock();
}

void
TalkingSkull::main() {
    wait_lock->lock();
    while (1) {
	wait_cond->wait(wait_lock);
	
	struct timespec next;
	nano_gettime(&next);

	for (size_t i = 0; i < n_ops; i++) {
	    struct timespec now;

	    nano_gettime(&now);
	    nano_add_usec(&next, usec_per_i);

	    if (nano_later_than(&next, &now)) {
		nano_sleep_until(&next);
		uint32_t decoded = 0;
		for (int j = 0; j < bytes_per_op; j++) decoded |= (ops[i*bytes_per_op + j]) << (8 * j);
		double pos = decoded / (double) op_bits * 100;
		update_pos(pos);
	    }
	}
    }
}

/* -------------------------------------------------------------- */

int
talking_skull_ops_to_filename(const char *fname, TalkingSkullOps *ops) {
    file_t *f;

    if ((f = file_open(fname, "rb")) == NULL) return -1;
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
