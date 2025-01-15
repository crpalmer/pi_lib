#include "pi.h"
#include <pico/stdlib.h>
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#include "pi-gpio.h"

#define NUM_GPIOS 40

typedef struct {
   pi_gpio_irq_handler_t f;
   void *arg;
} irq_handler_state_t;

typedef struct {
    unsigned gpio;
    enum { G_FREE = 0, G_IN, G_OUT, G_PWM } state;
    irq_handler_state_t irq_handler;
} gpio_t;

static gpio_t gpios[NUM_GPIOS];

#define N_PWM_SLICES   12

typedef struct {
    int channel_pins[2];
    unsigned hz;
    unsigned n_active;
} pwm_slice_t;

static pwm_slice_t pwm_slices[N_PWM_SLICES];

void pi_gpio_init()
{
    for (int i = 0; i < NUM_GPIOS; i++) {
	gpios[i].gpio = i;
    }

    for (int i = 0; i < N_PWM_SLICES; i++) {
        pwm_slices[i].channel_pins[0] = pwm_slices[i].channel_pins[1] = -1;
    }
}

int pi_gpio_get(unsigned gpio)
{
    if (gpios[gpio].state == G_FREE) gpios[gpio].state = G_IN;
    assert(gpios[gpio].state == G_IN);

    return gpio_get(gpio);
}

int pi_gpio_set(unsigned gpio, uint8_t value)
{
    if (gpios[gpio].state == G_FREE) gpios[gpio].state = G_OUT;
    assert(gpios[gpio].state == G_OUT);

    gpio_put(gpio, value);
    return 0;
}

int pi_gpio_set_pullup(unsigned gpio, unsigned updown)
{
    gpio_set_pulls(gpio, updown == PI_PUD_UP, updown == PI_PUD_DOWN);
    return 0;
}

int pi_gpio_set_direction(unsigned gpio, unsigned mode)
{
    gpio_init(gpio);
    pi_gpio_set_pullup(gpio, PI_PUD_OFF);
    gpio_set_dir(gpio, mode == PI_OUTPUT);
    gpios[gpio].state = (mode == PI_OUTPUT ? G_OUT : G_IN);
    return 0;
}

unsigned translate_events(uint32_t events)
{
    unsigned new_events = 0;

    if (events & GPIO_IRQ_EDGE_RISE) new_events = PI_GPIO_EVENT_RISING;
    if (events & GPIO_IRQ_EDGE_FALL) new_events = PI_GPIO_EVENT_FALLING;

    return new_events;
}

static void gpio_callback(uint gpio, uint32_t events) {
    irq_handler_state_t *irq = &gpios[gpio].irq_handler;

    if (irq->f) irq->f(irq->arg, gpio, translate_events(events));
}

int pi_gpio_set_irq_handler(unsigned gpio, pi_gpio_irq_handler_t irq_handler, void *irq_handler_arg)
{
    irq_handler_state_t *irq = &gpios[gpio].irq_handler;

    irq->f = irq_handler;
    irq->arg = irq_handler_arg;

    int saved = pico_pre_set_irq();
    gpio_set_irq_enabled_with_callback(gpio, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    pico_post_set_irq(saved);

    return 0;
}

int pi_gpio_servo(unsigned gpio, unsigned us)
{
    if (gpios[gpio].state == G_FREE) {
	if (pi_gpio_pwm_enable(gpio, 50) < 0) return -1;
	gpios[gpio].state = G_PWM;
    } else if (gpios[gpio].state != G_PWM) {
	return -1; 
    }

    int slice = pwm_gpio_to_slice_num(gpio);
    pwm_slice_t *s = &pwm_slices[slice];

    return pi_gpio_pwm_set_duty(gpio, us * s->hz / 1000.0 / 1000.0);
}

#define PWM_TOP_MAX 65534

static int
set_pwm_freq(int slice, unsigned freq)
{
    uint32_t source_hz = clock_get_hz(clk_sys);

    uint32_t div16_top = 16 * source_hz / freq;
    uint32_t top = 1;
    for (;;) {
        // Try a few small prime factors to get close to the desired frequency.
        if (div16_top >= 16 * 5 && div16_top % 5 == 0 && top * 5 <= PWM_TOP_MAX) {
            div16_top /= 5;
            top *= 5;
        } else if (div16_top >= 16 * 3 && div16_top % 3 == 0 && top * 3 <= PWM_TOP_MAX) {
            div16_top /= 3;
            top *= 3;
        } else if (div16_top >= 16 * 2 && top * 2 <= PWM_TOP_MAX) {
            div16_top /= 2;
            top *= 2;
        } else {
            break;
        }
    }
    if (div16_top < 16) {
        return -1; // freq too large
    } else if (div16_top >= 256 * 16) {
        return -1; // freq too small
    }
    pwm_hw->slice[slice].div = div16_top;
    pwm_hw->slice[slice].top = top;

    return 0;
}

int pi_gpio_pwm_enable(unsigned gpio, unsigned hz) {
    int slice = pwm_gpio_to_slice_num(gpio);
    int channel = pwm_gpio_to_channel(gpio);
    pwm_slice_t *s = &pwm_slices[slice];

    if (s->channel_pins[channel] >= 0) {
	return -1;
    }

    if (s->n_active > 0 && s->hz != hz) {
	return -1;
    }

    gpio_set_function(gpio, GPIO_FUNC_PWM);
    if (s->n_active == 0) {
	set_pwm_freq(slice, hz);
	s->hz = hz;
    }

    s->n_active++;
    s->channel_pins[channel] = gpio;

    return 0;
}

int pi_gpio_pwm_disable(unsigned gpio) {
    int slice = pwm_gpio_to_slice_num(gpio);
    pwm_slice_t *s = &pwm_slices[slice];

    if (s->channel_pins[0] == gpio) s->channel_pins[0] = -1;
    else if (s->channel_pins[1] == gpio) s->channel_pins[1] = -1;
    else return -1;

    s->n_active--;
    if (s->n_active == 0) {
	gpios[gpio].state = G_FREE;
        pwm_set_enabled(slice, false);
    }

    return 0;
}

int pi_gpio_pwm_set_duty(unsigned gpio, double pct)
{
    int slice = pwm_gpio_to_slice_num(gpio);
    int channel = pwm_gpio_to_channel(gpio);
    unsigned top = pwm_hw->slice[slice].top;

    pwm_set_chan_level(slice, channel, top * pct);
    pwm_set_enabled(slice, true);

    return 0;
}
