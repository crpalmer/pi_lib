#ifndef PI_PICO

#include <pigpio.h>

#else

#ifdef __cplusplus
extern "C" {
#endif

#define PI_PUD_OFF  0
#define PI_PUD_DOWN 1
#define PI_PUD_UP   2

#define PI_INPUT  0
#define PI_OUTPUT 1

int gpioInitialise();
int gpioInitialize();
int gpioRead(unsigned gpio);
int gpioWrite(unsigned gpio, unsigned value);
int gpioSetPullUpDown(unsigned gpio, unsigned updown);
int gpioSetMode(unsigned gpio, unsigned mode);
int gpioPWM(unsigned user_gpio, unsigned dutycycle);

#ifdef __cplusplus
};
#endif

#endif
