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
	canvas->fill(0, 0, 0);
	canvas->flush();
    }

    void on_touch(touch_event_t event) override {
	transform_position(&event.x, &event.y);
	canvas->fill(255, 255, 255, event.x / PIXELS_PER_TOUCH * PIXELS_PER_TOUCH, event.y / PIXELS_PER_TOUCH * PIXELS_PER_TOUCH, PIXELS_PER_TOUCH, PIXELS_PER_TOUCH);
	canvas->flush();
    }

    void on_released(touch_event_t event) override {
	transform_position(&event.x, &event.y);
	if (last_release_in_corner && nano_elapsed_ms_now(&released_at) < 300) {
	     canvas->fill(0, 0, 0);
	}
	last_release_in_corner = event.x < 64 && event.y < 64;
	if (last_release_in_corner) nano_gettime(&released_at);
    }

private:
    Display *display;
    Canvas *canvas;
    struct timespec released_at;
    bool last_release_in_corner = false;

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

