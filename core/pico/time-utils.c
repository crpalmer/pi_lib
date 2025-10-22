#include "pico/time.h"
#include "pico/types.h"
#include "pi.h"
#include "time-utils.h"

void nano_gettime(struct timespec *t) {
    absolute_time_t now = get_absolute_time();
    t->tv_sec = to_us_since_boot(now) / 1000 / 1000;
    t->tv_nsec = (to_us_since_boot(now) % (1000*1000)) * 1000;
}

void nano_sleep_until(struct timespec *t) {
    struct timespec now;
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
    busy_wait_us(us);
}
