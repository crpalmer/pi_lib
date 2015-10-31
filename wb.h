#ifndef __WB_H__
#define __WB_H__

#include <stdbool.h>

typedef struct wbS wb_t;

#define WB_OUTPUT(bank, pin) ((bank)*8 + pin)

wb_t *wb_new(void);

bool wb_get(wb_t *, unsigned pin);

unsigned wb_get_all(wb_t *);

void wb_set(wb_t *, unsigned pin, unsigned value);

void wb_destroy(wb_t *);

#endif
