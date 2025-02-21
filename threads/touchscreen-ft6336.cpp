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
#include "touchscreen-ft6336.h"

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

TouchscreenEventHandler::TouchscreenEventHandler(int i2c, int touch_delta, const char *name) : PiThread(name), i2c(i2c), touch_delta(touch_delta) {
    if (i2c_write_byte(i2c, G_MODE, G_MODE_POLLING) < 0) {
	fprintf(stderr, "Failed to set the g-mode\n");
    }

    lock = new PiMutex();
    cond = new PiCond();
    start();
}

void TouchscreenEventHandler::set_touch_state(bool is_touched) {
    lock->lock();
    this->is_touched = is_touched;
    cond->signal();
    lock->unlock();
}

void TouchscreenEventHandler::main() {
    while (1) {
	lock->lock();
	while (! is_touched) {
	    cond->wait(lock);
	}
	lock->unlock();

	report_touches();
    }
}

int TouchscreenEventHandler::report_touches() {
    uint8_t cur_n_touches;
    bool seen[MAX_TOUCHES] = { false, };
    int new_n_touches = n_touches;

    if (i2c_read_byte(i2c, TD_STATUS, &cur_n_touches) < 0) {
	fprintf(stderr, "Read of TD_STATUS failed.\n");
	return -1;
    }

    for (int i = 0; i < cur_n_touches; i++) {
	uint16_t x, y;

	if (read_pos(i2c, P1_X + i*P2_DELTA, &x) < 0 || read_pos(i2c, P1_Y + i*P2_DELTA, &y) < 0) {
	    fprintf(stderr, "Read of touch %d failed.\n", i);
	    return -1;
	}

	int j = 0;
	for (; j < n_touches; j++) {
	    if (is_same_touch_as(x, y, &touches[j])) {
		seen[j] = true;
		if (x != touches[j].x || y != touches[j].y) {
		    touches[j].x = x;
		    touches[j].y = y;
		    on_touch(touches[j]);
		}
		break;
	    }
	}

	if (j >= n_touches) {
	    touches[new_n_touches].id = touch_id++;
	    touches[new_n_touches].x  = x;
	    touches[new_n_touches].y  = y;
	    nano_gettime(&touches[new_n_touches].touch_at);
	    on_touch(touches[new_n_touches]);
	    new_n_touches++;
	}
    }

    int j = 0;
    for (int i = 0; i < new_n_touches; i++) {
	if (i > j) touches[j] = touches[i];
	if (i < n_touches && ! seen[i]) on_released(touches[i]);
	else j++;
    }

    n_touches = j;
    return n_touches;
}

int TouchscreenEventHandler::read_pos(int i2c, int reg, uint16_t *val) {
    uint8_t b1, b2;

    if (i2c_read_byte(i2c, reg+1, &b1) < 0 || i2c_read_byte(i2c, reg, &b2) < 0) return -1;
    *val = b1 | (((uint16_t) (b2 & 0x07)) << 8);
    return 1;
}

bool TouchscreenEventHandler::is_same_touch_as(int x, int y, touch_event_t *event) {
    return is_same_point(x, event->x) && is_same_point(y, event->y);
}

bool TouchscreenEventHandler::is_same_point(int a, int b) {
    return (a > b ? a - b : b - a) <= touch_delta;
}
