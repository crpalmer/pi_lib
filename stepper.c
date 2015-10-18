#include <stdio.h>
#include <malloc.h>
#include "gpio.h"
#include "mem.h"
#include "time-utils.h"
#include "util.h"

#include "stepper.h"

#define N_COILS STEPPER_N_COILS

#define MIN_STEP_MS	0

struct stepperS {
    int state;
    gpio_t *gpios;
    gpio_table_t gpio_table[N_COILS];
    int step_ms;
    struct timespec next_step;
};

static int states[N_COILS][N_COILS] = {
#if 1
    { 1, 1, 0, 0 },
    { 0, 1, 1, 0 },
    { 0, 0, 1, 1 },
    { 1, 0, 0, 1 },
#else
    { 1, 0, 0, 0 },
    { 0, 1, 0, 0 },
    { 0, 0, 1, 0 },
    { 0, 0, 0, 1 },
#endif
};

static gpio_table_t default_gpio_table[N_COILS] = {
    { "A", -1, 0 },
    { "B", -1, 0 },
    { "C", -1, 0 },
    { "D", -1, 0 },
};
stepper_t *
stepper_new(int gpios[N_COILS], int step_delay_ms)
{
    stepper_t *s = fatal_malloc(sizeof(*s));
    int i;

    for (i = 0; i < N_COILS; i++) {
        s->gpio_table[i] = default_gpio_table[i];
	s->gpio_table[i].gpio = gpios[i];
    }
    s->state = 0;
    s->step_ms = MIN_STEP_MS + step_delay_ms;
    nano_gettime(&s->next_step);

    s->gpios = gpio_new(s->gpio_table, N_COILS);

    return s;
}

void stepper_wait_complete(stepper_t *s)
{
    struct timespec now;

    nano_gettime(&now);
    if (! nano_later_than(&now, &s->next_step)) {
	ms_sleep(nano_elapsed_ms(&s->next_step, &now));
    }
}

static void
step(stepper_t *s)
{
    int i;

    stepper_wait_complete(s);

    for (i = 0; i < N_COILS; i++) {
	gpio_set_id(s->gpios, i, states[s->state][i]);
    }

    nano_gettime(&s->next_step);
    nano_add_ms(&s->next_step, s->step_ms);
}

void stepper_forward(stepper_t *s, unsigned n)
{
    while (n > 0) {
	s->state = (s->state + 1) % N_COILS;
	step(s);
	n--;
    }
}

void stepper_backward(stepper_t *s, unsigned n)
{
    while (n > 0) {
	s->state = (s->state ? s->state - 1 : N_COILS - 1);
	step(s);
	n--;
    }
}

void stepper_destroy(stepper_t *s)
{
    gpio_destroy(s->gpios);
    free(s);
}
