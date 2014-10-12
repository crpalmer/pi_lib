#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdbool.h>

typedef struct {
    const char *name;
    int         gpio;
    bool	initially_high;
} gpio_table_t;

typedef struct gpioS gpio_t;

gpio_t *
gpio_new(gpio_table_t *table, int n_table);

int
gpio_high(gpio_t *g, const char *name);
#define gpio_off(g, name) gpio_high(g, name)

void
gpio_high_id(gpio_t *g, size_t id);
#define gpio_off_id(g, id) gpio_high_id(g, id)

int
gpio_low(gpio_t *g, const char *name);
#define gpio_on(g, name) gpio_low(g, name);

void
gpio_low_id(gpio_t *g, size_t id);
#define gpio_on_id(g, id) gpio_low_id(g, id)

void
gpio_destroy(gpio_t *g);

#endif
