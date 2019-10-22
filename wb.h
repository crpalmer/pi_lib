#ifndef __WB_H__
#define __WB_H__

#ifdef __cplusplus

#include "io.h"

input_t *wb_get_input(unsigned pin);
output_t *wb_get_output(unsigned bank, unsigned pin);
output_t *wb_get_output(unsigned pin);

extern "C" {
#endif

#include <stdbool.h>

#define WB_PIN_MASK(pin) (1<<(pin))
#define WB_PIN_MASK_ALL (0x1fe)

typedef enum {
   WB_PULL_UP_NONE,
   WB_PULL_UP_DOWN,
   WB_PULL_UP_UP
} wb_pull_up_mode_t;

int wb_init(void);
int wb_init_v2(void);

bool wb_get(unsigned pin);

unsigned wb_get_all(void);
unsigned wb_get_all_with_debounce(unsigned debounce_ms);

void wb_set(unsigned bank, unsigned pin, unsigned value);

void wb_set_outputs(unsigned mask, unsigned values);

void wb_pwm(unsigned bank, unsigned pin, float duty);

void wb_pwm_freq(unsigned bank, unsigned pin, unsigned freq, float duty);

void wb_servo(unsigned bank, unsigned pin, unsigned pulse_ms);

void wb_set_pull_up(unsigned pin, wb_pull_up_mode_t mode);

unsigned wb_wait_for_pins_full(unsigned pins, unsigned values, unsigned debounce_ms, unsigned max_ms);

unsigned wb_wait_for_pins_timeout(unsigned pins, unsigned values, unsigned max_ms);

unsigned wb_wait_for_pins(unsigned pins, unsigned values);

bool wb_wait_for_pin_timeout(unsigned pin, unsigned value, unsigned max_ms);

void wb_wait_for_pin(unsigned pin, unsigned value);

#ifdef __cplusplus
};
#endif

#endif
