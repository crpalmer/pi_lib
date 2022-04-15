#ifndef __NES_H__
#define __NES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdio.h>

enum nes_button {
    NES_LEFT_RIGHT, NES_UP_DOWN, NES_SELECT, NES_START, NES_A, NES_B
};

typedef struct {
    enum nes_button button;
    int dir;	/* +1, 0, -1: 0 means released */
} nes_event_t;

int nes_read(nes_event_t *e, FILE *f);

int nes_read_legacy(nes_event_t *e, FILE *f);

#ifdef __cplusplus
};
#endif

#endif
