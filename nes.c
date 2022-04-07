#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <linux/input-event-codes.h>
#include "nes.h"

static void handle_key_event(struct input_event *e, nes_event_t *nes)
{
    switch(e->code) {
    case 0x131: nes->button = NES_A; break;
    case 0x132: nes->button = NES_B; break;
    case 0x138: nes->button = NES_SELECT; break;
    case 0x139: nes->button = NES_START; break;
    default: nes->button = -1;
    }
    nes->dir = (e->value != 0);
}

static void handle_abs_event(struct input_event *e, nes_event_t *nes)
{
    switch(e->code) {
    case ABS_X: nes->button = NES_LEFT_RIGHT; break;
    case ABS_Y: nes->button = NES_UP_DOWN; break;
    default: nes->button = -1;
    }

    switch(e->value) {
    case 0xff: nes->dir = +1; break;
    case 0x80: nes->dir = 0; break;
    case 0x00: nes->dir = -1; break;
    default: nes->dir = 0;
    }
}

int
nes_read(nes_event_t *nes, FILE *f)
{
    struct input_event e;

    if (fread(&e, sizeof(e), 1, f) == 1) {
	switch(e.type) {
	case EV_KEY: handle_key_event(&e, nes); break;
	case EV_ABS: handle_abs_event(&e, nes); break;
	default: return 0;
	}
	return 1;
    } else {
	return -1;
    }
}
