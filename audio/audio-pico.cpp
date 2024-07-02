#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "audio.h"
#include "audio-buffer.h"
#include "audio_i2s.pio.h"
#include "consoles.h"
#include "pi-threads.h"

#include "audio-pico.h"

#define SAMPLES_PER_BUFFER 256

static inline PIO PIOx(int pio) { return pio == 0 ? pio0 : pio1; }
static inline gpio_function GPIO_FUNC_PIOx(int pio) { return pio == 0 ? GPIO_FUNC_PIO0 : GPIO_FUNC_PIO1; }

static uint8_t dma_channel = -1;

static PiThread *blocked_thread = NULL;
static PiMutex *dma_mutex;
static int dma_pending_n;
static void *dma_pending = NULL;
static bool dma_started = false;
static uint32_t n_transfers[2];

static void dma_init() {
    dma_mutex = new PiMutex();
}

static inline void dma_start_transfer() {
    if (! dma_pending) {
n_transfers[0]++;
	static uint32_t zero[SAMPLES_PER_BUFFER] = {0, };
	dma_channel_transfer_from_buffer_now(dma_channel, &zero, SAMPLES_PER_BUFFER);
    } else {
n_transfers[1]++;
	dma_channel_transfer_from_buffer_now(dma_channel, dma_pending, dma_pending_n);
	dma_pending = NULL;
    }

    if (blocked_thread) {
	blocked_thread->resume_from_isr();
	blocked_thread = NULL;
    }
}

static void __isr __time_critical_func(dma_irq_handler)() {
    if (dma_irqn_get_channel_status(0, dma_channel)) {
        dma_irqn_acknowledge_channel(0, dma_channel);
        dma_start_transfer();
    }
}

static void dma_wait() {
    PiThread *me = PiThread::self();
    blocked_thread = me;
    me->pause();
}

static void dma_enqueue_data_locked(void *data, int n) {
    if (dma_pending) {
	assert(! blocked_thread);
	dma_wait();
        assert(dma_pending == NULL);
    }
    dma_pending_n = n;
    dma_pending = data;
    if (! dma_started) {
        irq_set_enabled(DMA_IRQ_0, true);
	dma_start_transfer();
	dma_started = true;
    }
}

AudioPico::AudioPico(int pio, int data_pin, int clock_pin_base) : pio(pio) {
    if (dma_channel != (uint8_t) -1) {
	consoles_fatal_printf("May only create a single AudioPico instance (dma_channel = %d)!\n", dma_channel);
    }

    dma_init();
    for (int i = 0; i < n_buffers; i++) buffers[i] = (uint8_t *) fatal_malloc(SAMPLES_PER_BUFFER * 4);

    gpio_set_function(data_pin, GPIO_FUNC_PIOx(pio));
    gpio_set_function(clock_pin_base + 0, GPIO_FUNC_PIOx(pio));
    gpio_set_function(clock_pin_base + 1, GPIO_FUNC_PIOx(pio));

    /* Set the PIO program */

    sm = pio_claim_unused_sm(PIOx(pio), true);
    unsigned offset = pio_add_program(PIOx(pio), &audio_i2s_program);

    audio_i2s_program_init(PIOx(pio), sm, offset, data_pin, clock_pin_base);

    //__mem_fence_release();

    /* Setup the DMA channel */

    dma_channel = dma_claim_unused_channel(true);

    dma_channel_config dma_config = dma_channel_get_default_config(dma_channel);
    channel_config_set_dreq(&dma_config, pio_get_dreq(PIOx(pio), sm, true));
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);

    dma_channel_configure(dma_channel, &dma_config, &PIOx(pio)->txf[sm], NULL, 0, false);

    irq_add_shared_handler(DMA_IRQ_0, dma_irq_handler, PICO_SHARED_IRQ_HANDLER_DEFAULT_ORDER_PRIORITY);
    dma_irqn_set_channel_enabled(0, dma_channel, true);

    /* Enable I2S */
    config_clocks();
    pio_sm_set_enabled(PIOx(pio), sm, true);
}

bool AudioPico::configure(AudioConfig *config) {
    if (config->get_num_channels() != 2 || config->get_rate() != 44100 || config->get_bytes_per_sample() != 2) {
	consoles_fatal_printf("Audio format is not currently compatible: %d channels, %d khz %d bytes per sample.\n44100 hz, 16 bit signed.\n", config->get_num_channels(), config->get_rate(), config->get_bytes_per_sample());
    }

    int old_rate = rate;

    num_channels = config->get_num_channels();
    rate = config->get_rate();
    bytes_per_sample = config->get_bytes_per_sample();

    if (old_rate != rate) {
	config_clocks();
    }

    return true;
}

void AudioPico::config_clocks() {
    uint32_t clock_freq = clock_get_hz(clk_sys);
    assert(clock_freq < 0x40000000);
    uint32_t divider = clock_freq * 4 / get_rate();
    assert(divider < 0x1000000);
    pio_sm_set_clkdiv_int_frac(PIOx(pio), sm, divider >> 8, divider & 0xff);
}

size_t AudioPico::get_recommended_buffer_size() {
    return SAMPLES_PER_BUFFER * get_bytes_per_sample() * get_num_channels();
};

bool AudioPico::play(void *data_vp, size_t n) {
    uint8_t *data = (uint8_t *) data_vp;

    dma_mutex->lock();

    for (size_t i = 0; i < n; ) {
	size_t n_to_copy = SAMPLES_PER_BUFFER > (n - i)/4 ? (n - i)/4 : SAMPLES_PER_BUFFER;

	memcpy(buffers[next_buffer], &data[i], n_to_copy * 4);
	dma_enqueue_data_locked(buffers[next_buffer], n_to_copy);

	next_buffer = (next_buffer + 1) % n_buffers;
	i += n_to_copy * 4;
    }

    dma_mutex->unlock();

    return true;
}

void AudioPico::disable() {
    dma_mutex->lock();
    if (dma_pending) dma_wait();
    irq_set_enabled(DMA_IRQ_0, false);
    dma_started = false;
    dma_mutex->unlock();
}

