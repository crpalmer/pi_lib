#ifndef __PI_GPIO_H__
#define __PI_GPIO_H__

#include <stdint.h>
#include <assert.h>

#define PI_PUD_OFF  0
#define PI_PUD_DOWN 1
#define PI_PUD_UP   2

#define PI_INPUT  0
#define PI_OUTPUT 1

#define PI_GPIO_EVENT_RISING  1
#define PI_GPIO_EVENT_FALLING 2
#ifdef __cplusplus
extern "C" {
#endif

void pi_gpio_init();
int pi_gpio_get(unsigned gpio);
int pi_gpio_set(unsigned gpio, uint8_t value);
int pi_gpio_set_pullup(unsigned gpio, unsigned updown);
int pi_gpio_set_direction(unsigned gpio, unsigned direction);
int pi_gpio_servo(unsigned gpio, unsigned ms);
int pi_gpio_pwm_enable(unsigned gpio, unsigned freq_hz);
int pi_gpio_pwm_disable(unsigned gpio);
int pi_gpio_pwm_set_duty(unsigned gpio, double pct);

typedef void (*pi_gpio_irq_handler_t)(void *arg, unsigned gpio, unsigned events);
int pi_gpio_set_irq_handler(unsigned gpio, pi_gpio_irq_handler_t irq_handler, void *irq_handler_arg);

#ifndef NO_PIGPIO_EMULATION

/* Preserve the pigpio names for compatibility outside of this
 * library, but build the library without them to keep the code
 * clean.
 */

static inline int gpioRead(unsigned gpio) { return pi_gpio_get(gpio); }
static inline int gpioWrite(unsigned gpio, uint8_t value) { return pi_gpio_set(gpio, value); }
static inline int gpioSetPullUpDown(unsigned gpio, unsigned updown) { return pi_gpio_set_pullup(gpio, updown); }
static inline int gpioSetMode(unsigned gpio, unsigned mode) { return pi_gpio_set_direction(gpio, mode); }
static inline int gpioPWM(unsigned gpio, unsigned dutycycle) { assert(0); return -1; }

/* TODO */

// int spiWrite(int, char *, unsigned);
// int spiOpen(int, int, int);

#endif
 
#ifdef __cplusplus
};
#endif

#endif
