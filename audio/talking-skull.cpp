#include <stdio.h>
#include <stdlib.h>
#include "consoles.h"
#include "mem.h"
#include "time-utils.h"
#include "talking-skull.h"

TalkingSkullFileOps::TalkingSkullFileOps(const char *fname) {
    if ((f = fopen(fname, "r")) == NULL) {
	consoles_fatal_printf("Could not open: %s\n", fname);
    }
    fscanf(f, "%d", &usec_per_i);
}

TalkingSkullFileOps::~TalkingSkullFileOps() {
    fclose(f);
}

bool TalkingSkullFileOps::next(double *pos) {
    return fscanf(f, "%lg", pos) == 1;
}

/* -------------------------------------------------------------- */

TalkingSkullVsaOps::TalkingSkullVsaOps(const char *fname) {
    if ((f = fopen(fname, "r")) == NULL) {
	consoles_fatal_printf("Could not open %s\n", fname);
    }
}

TalkingSkullVsaOps::~TalkingSkullVsaOps() {
    fclose(f);
}
    
/* -------------------------------------------------------------- */

bool TalkingSkullVsaOps::next(double *pos) {
    char buf[128];

    if (! fgets(buf, sizeof(buf), f)) return false;
    *pos = atoi(buf) / 254.0 * 100;

    return true;
}

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
    FILE *f;

    if ((f = fopen(fname, "w")) == NULL) return -1;
    int ret = talking_skull_ops_to_file(f, ops);
    fclose(f);

    return ret;
}

int
talking_skull_ops_to_file(FILE *f, TalkingSkullOps *ops) {
    fprintf(f, "%d", ops->get_usec_per_i());
    double pos;
    for (int i = 0; ops->next(&pos); i++) {
	fprintf(f, "%c%g", i % 10 == 0 ? '\n' : ' ', pos);
    }
    fclose(f);

    return 0;
}
