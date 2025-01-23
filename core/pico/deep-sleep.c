#include "pi.h"
#include "pico/stdlib.h"
#include "pico/sleep.h"

void pico_enter_deep_sleep_until(int gpio) {
        printf("Switching to XOSC\n");
        uart_default_tx_wait_blocking();

        // Set the crystal oscillator as the dormant clock source, UART will be reconfigured from here
        // This is necessary before sending the pico into dormancy
        sleep_run_from_xosc();

        printf("Going dormant until GPIO %d goes edge high\n", gpio);
        uart_default_tx_wait_blocking();

        // Go to sleep until we see a low edge on GPIO #gpio
        sleep_goto_dormant_until_pin(gpio, true, false);

        // Re-enabling clock sources and generators.
        sleep_power_up();
}
