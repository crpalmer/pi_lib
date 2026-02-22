#include "pico/time.h"
#include "pico/types.h"
#include "pi.h"
#include "time-utils.h"

void nano_gettime(nano_time_t *t) {
    *t = get_absolute_time();
}

void nano_sleep_until(nano_time_t *t) {
    nano_time_t now;
    int ms;

    nano_gettime(&now);
    ms = nano_elapsed_ms(t, &now);
    if (ms > 0) sleep_ms(ms);
}

void normal_sleep(unsigned ms) {
    sleep_ms(ms);
}
    
static sleep_fn_t sleep_fn = normal_sleep;

void ms_sleep(unsigned ms) {
    (*sleep_fn)(ms);
}

void pico_set_sleep_fn(sleep_fn_t new_sleep_fn) {
    sleep_fn = new_sleep_fn; 
}

void us_sleep(unsigned us) {
    unsigned ms = us / 1000;
    if (ms >= 1) {
	sleep_ms(ms);
	us -= ms * 1000;
    }
    busy_wait_us(us);
}
