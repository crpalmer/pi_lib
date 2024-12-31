#ifndef __DEEP_SLEEP_H__
#define __DEEP_SLEEP_H__

#ifdef __cplusplus
extern "C" {
#endif

void pico_enter_deep_sleep_until(int gpio);

#ifdef __cplusplus
};
#endif

#endif
