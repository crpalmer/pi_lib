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

class EventHandler : public TouchscreenEventHandler {
public:
    EventHandler(int i2c) : TouchscreenEventHandler(i2c) {
    }

    void on_touch(touch_event_t event) override {
	int x = event.x;
	int y = event.y;
	transform_position(&x, &y);
	printf("touched  %d - %d, %d (%d ms)\n", event.id, x, y, nano_elapsed_ms_now(&event.touch_at));
    }

    void on_released(touch_event_t event) override {
	int x = event.x;
	int y = event.y;
	transform_position(&x, &y);
	printf("released %d - %d, %d (%d ms)\n", event.id, x, y, nano_elapsed_ms_now(&event.touch_at));
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

