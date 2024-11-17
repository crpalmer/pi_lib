#include "pi.h"
#include "pico/time.h"
#include "pico/types.h"
#include "distance.h"

int
ping_distance_cm(Output *trigger, Input *echo) {
    trigger->set(0);
    sleep_us(2);
    trigger->set(1);
    sleep_us(10);
    trigger->set(0);

    absolute_time_t start = get_absolute_time();
    absolute_time_t timeout = delayed_by_us(start, 23332);
    absolute_time_t now = start;

    if (start > timeout) {
	// Clocks wrapped, just call it an error
	return -2;
    }

    //while (echo->get() && (now = get_absolute_time()) < timeout) {}
    while (echo->get() && (now = get_absolute_time()) < timeout) {}

    if (now >= timeout) return -1;

    start = now;
    timeout = delayed_by_us(start, 23332);
    while (! echo->get() && (now = get_absolute_time()) < timeout) {}

    // potential echo reverbs
    ms_sleep(10);

    return (now - start) / 29 / 2;
}
