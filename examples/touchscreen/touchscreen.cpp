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

#include "../display-common.h"

#define PIXELS_PER_TOUCH 5

class EventHandler : public TouchscreenEventHandler {
public:
    EventHandler(int i2c) : TouchscreenEventHandler(i2c) {
	display = create_display(USE_ST7796S);
	canvas = display->create_canvas(true);
	w = canvas->get_width();
	h = canvas->get_height();
	n_bits = (w/PIXELS_PER_TOUCH+1)*(h/PIXELS_PER_TOUCH+1);
	set = (uint8_t *) fatal_malloc(n_bits / 8 + 1);
	clear_screen();
    }

    void on_touch(touch_event_t event) override {
	transform_position(&event.x, &event.y);
	int x = event.x / PIXELS_PER_TOUCH;
	int y = event.y / PIXELS_PER_TOUCH;
	int bit = x + y * (w / PIXELS_PER_TOUCH);
	if (! (set[bit / 8] & (1 << (bit % 8)))) {
	    canvas->fill(255, 255, 255, x * PIXELS_PER_TOUCH, y * PIXELS_PER_TOUCH, PIXELS_PER_TOUCH, PIXELS_PER_TOUCH);
	    canvas->flush();
	    set[bit / 8] |= (1 << (bit % 8));
	}
    }

    void on_released(touch_event_t event) override {
	transform_position(&event.x, &event.y);
	if (last_release_in_corner && nano_elapsed_ms_now(&released_at) < 300) {
	    clear_screen();
	}
	last_release_in_corner = event.x < (w/8) && event.y < (h/8);
	if (last_release_in_corner) nano_gettime(&released_at);
    }

private:
    Display *display;
    Canvas *canvas;
    int w, h;

    uint8_t *set;
    int n_bits;

    struct timespec released_at;
    bool last_release_in_corner = false;

    void transform_position(int *x, int *y) {
	uint16_t tmp = *x;
	*x = *y;
	*y = tmp;

	*y = 320 - *y;
    }

    void clear_screen() {
	canvas->fill(0, 0, 0);
	canvas->flush();
	memset(set, 0, n_bits / 8 + 1);
    }
};

void threads_main(int argc, char **argv) {
    ms_sleep(1000);
    printf("Starting.\n");

    i2c_init_bus();

    int i2c = i2c_open(1, 0x38);
    if (i2c < 0) {
	perror("i2c_open");
	pi_abort();
    }

    EventHandler *event_handler = new EventHandler(i2c);
    GPInput *interrupt = new GPInput(4);
    new TouchscreenInterruptNotifier(interrupt, event_handler);
}

int main(int argc, char **argv) {
    pi_init_with_threads(threads_main, argc, argv);
}

