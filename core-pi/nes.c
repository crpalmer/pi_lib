#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "nes.h"

#include <linux/input.h>
#include <linux/input-event-codes.h>

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
nes_read(nes_event_t *nes, file_t *f)
{
    struct input_event e;

    if (file_read(f, &e, sizeof(e)) == sizeof(e)) {
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

static enum nes_button
handle_legacy_button(unsigned char *buf)
{
    switch(buf[7]) {
    case 1: return NES_A;
    case 2: return NES_B;
    case 8: return NES_START;
    case 9: return NES_SELECT;
    }
    return -1;
}

static int
handle_legacy_arrow_dir(unsigned char *buf)
{
    if (buf[4] == 0) return 0;
    else return buf[5] == 0x7f ? 1 : -1;
}

static enum nes_button
handle_legacy_arrow(unsigned char *buf)
{
    return buf[7] == 0 ? NES_LEFT_RIGHT : NES_UP_DOWN;
}

int
nes_read_legacy(nes_event_t *nes, file_t *f)
{
    unsigned char buf[8];

    if (file_read(f, buf, sizeof(buf)) != sizeof(buf)) return -1;
    switch(buf[6]) {
    case 1:
	nes->button = handle_legacy_button(buf);
	nes->dir = buf[4];
	break;
    case 2:
	nes->button = handle_legacy_arrow(buf);
	nes->dir = handle_legacy_arrow_dir(buf);
	break;
    }
    return 1;
}
