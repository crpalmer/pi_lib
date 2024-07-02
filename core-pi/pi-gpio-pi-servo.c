#include "pi.h"
#include "pi-gpio.h"

#ifdef PLATFORM_linux

#include "consoles.h"
int pi_gpio_servo(unsigned gpio, unsigned ms) {
    consoles_fatal_printf("%s not supported.\n", __func__);
    return 0;
}

#else

#include <pigpio.h>
int pi_gpio_servo(unsigned gpio, unsigned ms) {
    return gpioServo(gpio, ms);
}

#endif
