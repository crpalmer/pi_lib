#ifndef __PI_PLATFORM_H__
#define __PI_PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*sleep_fn_t)(unsigned ms);

void pico_set_sleep_fn(sleep_fn_t new_sleep_fn);
void pico_set_rtc(time_t seconds_since_epoch);

int pico_pre_set_irq();
void pico_post_set_irq(int saved_value);
void pico_set_irq_hook_functions(int (*pre)(), void (*post)(int));

#ifdef __cplusplus
};
#endif

#endif
