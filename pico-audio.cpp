#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico-audio.h"
#include "pico-audio.pio.h"

#define PICO_AUDIO_DATA_PIN 26
#define PICO_AUDIO_CLOCK_PIN_BASE 27

#define GPIO_FUNC_PIOx(pio) (pio == pio0 ? GPIO_FUNC_PIO0 : GPIO_FUNC_PIO1)

PicoAudio::PicoAudio(PIO pio, int sm)
{
    this->pio = pio;
    this->sm  = sm;

    gpio_set_function(PICO_AUDIO_DATA_PIN, GPIO_FUNC_PIOx(pio));
    gpio_set_function(PICO_AUDIO_CLOCK_PIN_BASE, GPIO_FUNC_PIOx(pio));
    gpio_set_function(PICO_AUDIO_CLOCK_PIN_BASE+1, GPIO_FUNC_PIOx(pio));

    pio_sm_claim(pio, sm);
printf("pio_sm_claim");

    uint offset = pio_add_program(pio, &audio_pio_program);
printf("pio_add_program");

    audio_pio_program_init(pio, sm, offset, PICO_AUDIO_DATA_PIN, PICO_AUDIO_CLOCK_PIN_BASE);
printf("audio_program_init");

    set_frequency(40000);

    pio_sm_set_enabled(pio, sm, true);
printf("pio_set_enabled");
}

void PicoAudio::set_frequency(int freq)
{
    uint32_t system_clock_frequency = clock_get_hz(clk_sys);
    uint32_t divider = system_clock_frequency * 4 / freq; // avoid arithmetic overflow
    pio_sm_set_clkdiv_int_frac(pio, sm, divider >> 8u, divider & 0xffu);
printf("freq %d %d\n", divider >> 8u, divider & 0xffu);
}

void PicoAudio::play(unsigned char *samples, int n_samples)
{
    for (int i = 0; i < n_samples; i++) {
	pio_sm_put_blocking(pio, sm, samples[i] << (3*8));
    }
}
