#ifndef __PI_H__
#define __PI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>

void pi_init(void);
void pi_init_no_reboot();

char *pi_readline(char *buf, size_t buflen);

void pi_reboot_bootloader();

typedef void (*sleep_fn_t)(unsigned ms);

void pico_set_sleep_fn(sleep_fn_t new_sleep_fn);

void pico_set_rtc(time_t seconds_since_epoch);

#ifdef __cplusplus
};
#endif

#endif