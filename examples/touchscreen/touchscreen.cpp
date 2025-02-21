/* NOTE:
 *
 * You must set the reset pin to 3.3v.  I am just connecting it to the 3.3v line to raise it to high.
 */

#include "pi.h"
#include <string.h>
#include "gp-input.h"
#include "gp-output.h"
#include "i2c.h"
#include "time-utils.h"
#include "thread-interrupt-notifier.h"

#define GEST_ID		0x01
#define TD_STATUS	0x02
#define P1_X		0x03
#define P1_Y		0x05
#define P1_WEIGHT	0x07
#define P1_MISC		0x08
#define P2_DELTA	6
#define TH_GROUP	0x80
#define G_MODE		0xa4

#define TOUCH_THRESHOLD	7
#define G_MODE_POLLING	0
#define G_MODE_TRIGGER	1

#define DEBUG 0

#if DEBUG
static void dump_byte_reg(int i2c, const char *str, int reg) {
    uint8_t value;

    if (i2c_read_byte(i2c, reg, &value) < 0) {
	fprintf(stderr, "failed to read byte reg 0x%02x: ", reg);
	perror("i2c_read_byte");
    } else {
	printf("%s [0x%02x] = 0x%02x\n", str, reg, value);
    }
}

static void dump_word_reg(int i2c, const char *str, int reg) {
    uint16_t value;

    if (i2c_read_word(i2c, reg, &value) < 0) {
	fprintf(stderr, "failed to read byte reg 0x%02x: ", reg);
	perror("i2c_read_byte");
    } else {
	printf("%s [0x%02x] = 0x%04x\n", str, reg, value);
    }
}
#endif

class TouchscreenEventHandler : public PiThread {
public:
    TouchscreenEventHandler(int i2c, const char *name = "event-handler") : PiThread(name), i2c(i2c) {
	if (i2c_write_byte(i2c, G_MODE, G_MODE_POLLING) < 0) {
	    fprintf(stderr, "Failed to set the g-mode\n");
	}

	lock = new PiMutex();
	cond = new PiCond();
	start();
    }

    void set_touch_state(bool is_touched) {
	lock->lock();
	this->is_touched = is_touched;
	cond->signal();
	lock->unlock();
    }

    void main() {
	while (1) {
	    lock->lock();
	    while (! is_touched) {
		cond->wait(lock);
	    }
	    lock->unlock();

	    report_touches();
	}
    }

    virtual void on_touch(int x, int y) = 0;

private:
    int i2c;
    PiMutex *lock;
    PiCond *cond;
    bool is_touched = false;
    uint8_t last_n_touches = 0;
    uint16_t last_touches[2][2];

    int report_touches() {
	uint8_t n_touches;

	if (i2c_read_byte(i2c, TD_STATUS, &n_touches) < 0) {
	    fprintf(stderr, "Read of TD_STATUS failed.\n");
	    return -1;
	}

	if (n_touches == 0 && last_n_touches > 0) {
	    printf("No touches\n");
	}

	uint16_t cur_touches[2][2];

	for (int i = 0; i < n_touches; i++) {
	    uint16_t x, y;
	    uint8_t weight, misc;

	    if (read_pos(i2c, P1_X + i*P2_DELTA, &x) < 0 ||
		read_pos(i2c, P1_Y + i*P2_DELTA, &y) < 0 ||
		i2c_read_byte(i2c, P1_WEIGHT + i*P2_DELTA, &weight) < 0 ||
		i2c_read_byte(i2c, P1_MISC + i*P2_DELTA, &misc) < 0) {
		fprintf(stderr, "Read of touch %d failed.\n", i);
		return -1;
	    }
	    cur_touches[i][0] = x;
	    cur_touches[i][1] = y;
	    if (! is_current_touch(x, y)) on_touch(x,y);
	}
 
	memcpy(last_touches, cur_touches, sizeof(last_touches));
	last_n_touches = n_touches;
	return n_touches;
    }

    int read_pos(int i2c, int reg, uint16_t *val) {
	uint8_t b1, b2;

	if (i2c_read_byte(i2c, reg+1, &b1) < 0 || i2c_read_byte(i2c, reg, &b2) < 0) return -1;
	*val = b1 | (((uint16_t) (b2 & 0x07)) << 8);
	return 1;
    }

    bool is_current_touch(int x, int y) {
	for (int i = 0; i < last_n_touches; i++) {
	    if (last_touches[i][0] == x && last_touches[i][1] == y) return true;
	}
	return false;
    }
};

class TouchscreenInterruptNotifier : public ThreadInterruptNotifier {
public:
    TouchscreenInterruptNotifier(Input *interrupt, TouchscreenEventHandler *event_handler) : interrupt(interrupt), event_handler(event_handler) {
        interrupt->set_notifier(this);
    }

    void on_change_safe() {
	event_handler->set_touch_state(interrupt->get());
    }

private:
    Input *interrupt;
    TouchscreenEventHandler *event_handler;
};

class EventHandler : public TouchscreenEventHandler {
public:
    EventHandler(int i2c) : TouchscreenEventHandler(i2c) {
    }

    void on_touch(int x, int y) override {
	transform_position(&x, &y);
	printf("%d, %d\n", x, y);
    }

private:
    void transform_position(int *x, int *y) {
	uint16_t tmp = *x;
	*x = *y;
	*y = tmp;

	*y = 320 - *y;
    }
};

void threads_main(int argc, char **argv) {
    ms_sleep(1000);
    printf("Starting.\n");

    i2c_init_bus();

#if DEBUG
    printf("Scanning:\n");
    for (int i = 0; i < 0x80; i++) {
	if (i2c_exists(1, i)) printf("  0x%02x exists\n", i);
    }
    printf("Done scanning.\n");
#endif

    int i2c = i2c_open(1, 0x38);
    if (i2c < 0) {
	perror("i2c_open");
	pi_abort();
    }

#if DEBUG
    dump_byte_reg(i2c, "dev_mode", 0x00);
    dump_byte_reg(i2c, "th_group", 0x80);
    dump_byte_reg(i2c, "th_diff ", 0x85);
    dump_byte_reg(i2c, "ctrl    ", 0x86);
    dump_byte_reg(i2c, "t->monit", 0x87);
    dump_byte_reg(i2c, "a-rate  ", 0x88);
    dump_byte_reg(i2c, "m-rate  ", 0x89);
    dump_byte_reg(i2c, "radians ", 0x91);
    dump_byte_reg(i2c, "o-lright", 0x92);
    dump_byte_reg(i2c, "o-updown", 0x93);
    dump_byte_reg(i2c, "d-lright", 0x94);
    dump_byte_reg(i2c, "d-updown", 0x95);
    dump_byte_reg(i2c, "d-zoom  ", 0x96);
    dump_word_reg(i2c, "lib-vers", 0xa1);
    dump_byte_reg(i2c, "chip-sel", 0xa3);
    dump_byte_reg(i2c, "g-mode  ", 0xa4);
    dump_byte_reg(i2c, "pwr-mode", 0xa5);
    dump_byte_reg(i2c, "firm-id ", 0xa6);
    dump_byte_reg(i2c, "focal-id", 0xa8);
    dump_byte_reg(i2c, "code-id ", 0xaf);
    dump_byte_reg(i2c, "state   ", 0xbc);
#endif

    if (i2c_write_byte(i2c, TH_GROUP, TOUCH_THRESHOLD) < 0) {
	fprintf(stderr, "Failed to set the touch threshold\n");
    }

    EventHandler *event_handler = new EventHandler(i2c);
    GPInput *interrupt = new GPInput(4);
    new TouchscreenInterruptNotifier(interrupt, event_handler);
}

int main(int argc, char **argv) {
    pi_init_with_threads(threads_main, argc, argv);
}

