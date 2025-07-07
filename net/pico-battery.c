#include "pi.h"
#include "hardware/adc.h"
#include "pico-battery.h"

#if CYW43_USES_VSYS_PIN
#include "pico/cyw43_arch.h"
#endif

#define ADC0_PIN		26
#define N_SAMPLES_TO_SKIP	3
#define N_SAMPLES		3

bool pico_is_on_battery() {
#if defined CYW43_WL_GPIO_VBUS_PIN
    return ! cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#elif defined PICO_VBUS_PIN
    gpio_set_function(PICO_VBUS_PIN, GPIO_FUNC_SIO);
    reutrn ! gpio_get(PICO_VBUS_PIN);
    return PICO_OK;
#else
    return false;
#endif
}

double pico_get_vsys() {
#ifndef PICO_VSYS_PIN
    return 0;
#else
#if CYW43_USES_VSYS_PIN
    cyw43_thread_enter();
    // Make sure cyw43 is awake
    cyw43_arch_gpio_get(CYW43_WL_GPIO_VBUS_PIN);
#endif

    // setup adc
    adc_gpio_init(PICO_VSYS_PIN);
    adc_select_input(PICO_VSYS_PIN - ADC0_PIN);
 
    adc_fifo_setup(true, false, 0, false, false);
    adc_run(true);

    // We seem to read low values initially - this seems to fix it
    int ignore_count = N_SAMPLES_TO_SKIP;
    while (!adc_fifo_is_empty() || ignore_count-- > 0) {
        (void)adc_fifo_get_blocking();
    }

    // read vsys
    uint32_t vsys = 0;
    for(int i = 0; i < N_SAMPLES; i++) {
        uint16_t val = adc_fifo_get_blocking();
        vsys += val;
    }

    adc_run(false);
    adc_fifo_drain();

    vsys /= N_SAMPLES;
#if CYW43_USES_VSYS_PIN
    cyw43_thread_exit();
#endif
    // Generate voltage
    const float conversion_factor = 3.3f / (1 << 12);
    return vsys * 3 * conversion_factor;
#endif
}
