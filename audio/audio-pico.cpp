#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "pico-audio.pio.h"
#include "audio.h"
#include "audio-buffer.h"

#define GPIO_FUNC_PIOx(pio) (pio == 0 ? GPIO_FUNC_PIO0 : GPIO_FUNC_PIO1)
#define PIO(pio) (pio == 0 ? pio0 : pio1)

AudioPico::AudioPico(int data_pin, int clock_pin, int pio, int sm) : pio(pio), sm(sm) {
    gpio_set_function(data_pin, GPIO_FUNC_PIOx(pio));
    gpio_set_function(clock_pin, GPIO_FUNC_PIOx(pio));
    gpio_set_function(clock_pin+1, GPIO_FUNC_PIOx(pio));

    pio_sm_claim(PIO(pio), sm);

    uint offset = pio_add_program(PIO(pio), &audio_pio_program);

    audio_pio_program_init(PIO(pio), sm, offset, data_pin, clock_pin);

    set_frequency(44100);

    pio_sm_set_enabled(PIO(pio), sm, true);
}

void AudioPico::set_frequency(int freq) {
    uint32_t system_clock_frequency = clock_get_hz(clk_sys);
    uint32_t divider = system_clock_frequency * 4 / freq; // avoid arithmetic overflow
    pio_sm_set_clkdiv_int_frac(PIO(pio), sm, divider >> 8u, divider & 0xffu);
}

bool AudioPico::play(void *data, size_t n) {
    uint32_t val;
    AudioBuffer *buffer = new AudioBuffer(new BufferBuffer(data, n), config);

    while (buffer->next(&val)) {
	pio_sm_put_blocking(PIO(pio), sm, val);
    }

    return true;
}
