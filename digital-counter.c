#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "wb.h"
#include "digital-counter.h"

struct digital_counterS {
    int bank;
    int inc, dec, reset;
    int value;
};

#define RESET_PAUSE 100
#define PAUSE 10

digital_counter_t *
digital_counter_new(int bank, int inc, int dec, int reset)
{
    digital_counter_t *dc = malloc(sizeof(digital_counter_t));

    dc->bank = bank;
    dc->inc = inc;
    dc->dec = dec;
    dc->reset = reset;
    digital_counter_reset(dc);

    return dc;
}

void
digital_counter_add(digital_counter_t *dc, int n)
{
    digital_counter_set(dc, dc->value + n);
}

void
digital_counter_set(digital_counter_t *dc, int new_value)
{
    while (new_value > dc->value) {
	wb_set(dc->bank, dc->dec, 1);
	ms_sleep(PAUSE);
	wb_set(dc->bank, dc->dec, 0);
	ms_sleep(PAUSE);
	dc->value++;
    }

    while (new_value < dc->value) {
	wb_set(dc->bank, dc->inc, 1);
	ms_sleep(PAUSE);
	wb_set(dc->bank, dc->inc, 0);
	ms_sleep(PAUSE);
	dc->value--;
    }
}

void digital_counter_reset(digital_counter_t *dc)
{
    dc->value = 0;
    wb_set(dc->bank, dc->reset, 1);
    ms_sleep(RESET_PAUSE);
    wb_set(dc->bank, dc->reset, 0);
}

void digital_counter_free(digital_counter_t *dc)
{
    free(dc);
}
