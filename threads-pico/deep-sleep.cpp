#include "pi.h"
#include "deep-sleep.h"

DeepSleeper::DeepSleeper(int wakeup_gpio, int sleep_after_ms, const char *thread_name) : PiThread(thread_name), wakeup_gpio(wakeup_gpio), sleep_after_ms(sleep_after_ms) {
    prod();
    check_ms = (sleep_after_ms + 9) / 10;	/* 10% possible over wake time */

    start();
}

void DeepSleeper::prod() {
    nano_gettime(&last_press);
}

void DeepSleeper::main(void) {
   while (1) {
       ms_sleep(check_ms);
       if (nano_elapsed_ms_now(&last_press) >= sleep_after_ms) {
	    pre_sleep();
	    printf("Going to sleep on gpio %d...\n", wakeup_gpio);
	    pico_enter_deep_sleep_until(wakeup_gpio);
	    post_sleep();
       }
   }
}
