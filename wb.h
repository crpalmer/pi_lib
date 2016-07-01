#ifndef __WB_H__
#define __WB_H__

#include <stdbool.h>

typedef struct wbS wb_t;

typedef enum {
   WB_PULL_UP_NONE,
   WB_PULL_UP_DOWN,
   WB_PULL_UP_UP
} wb_pull_up_mode_t;

#define WB_OUTPUT(bank, pin) (((bank-1))*8 + (pin-1))

int wb_init(void);

bool wb_get(unsigned pin);

unsigned wb_get_all(void);

void wb_set(unsigned pin, unsigned value);

void wb_pwm(unsigned pin, float duty);

void wb_pwm_freq(unsigned pin, unsigned freq, float duty);

void wb_servo(unsigned pin, unsigned pulse_ms);

void wb_set_pull_up(unsigned pin, wb_pull_up_mode_t mode);

#endif
