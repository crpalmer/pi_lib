#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mem.h"
#include "file.h"
#include "string-utils.h"
#include "gpio.h"
#include "externals/PIGPIO/pigpio.h"

#define GPIO_DIR_PREFIX "/sys/class/gpio/gpio"
#define EXPORT_FNAME "/sys/class/gpio/export"
#define UNEXPORT_FNAME "/sys/class/gpio/unexport"

#define HIGH_VALUE 0
#define LOW_VALUE 1

struct gpioS {
     gpio_table_t *table;
     int           n_table;
};

gpio_t *
gpio_new(gpio_table_t *table, int n_table)
{
     int i;
     gpio_t *g;

     if (gpioInitialise() < 0) return NULL;

     g = fatal_malloc(sizeof(*g));
     g->table = table;
     g->n_table = n_table;

     for (i = 0; i < g->n_table; i++) {
	if (g->table[i].initially_high == GPIO_IS_INPUT) {
	    gpioSetMode(g->table[i].gpio, PI_INPUT);
	} else {
	    gpioSetMode(g->table[i].gpio, PI_OUTPUT);
	    gpioWrite(g->table[i].gpio, g->table[i].initially_high ? LOW_VALUE : HIGH_VALUE);
	}
     }

     return g;
}

static bool
find_id(gpio_t *g, const char *name, size_t *id)
{
     size_t i;

     for (i = 0; i < g->n_table; i++) {
	if (strcmp(g->table[i].name, name) == 0) {
	     *id = i;
	     return true;
	}
    }
    return false;
}

static int
set_gpio(gpio_t *g, const char *name, int value)
{
    size_t id;

    if (! find_id(g, name, &id)) return -1;
    return gpioWrite(g->table[id].gpio, value);
}

int
gpio_low(gpio_t *g, const char *name)
{
     return set_gpio(g, name, LOW_VALUE);
}

int
gpio_high(gpio_t *g, const char *name)
{
     return set_gpio(g, name, HIGH_VALUE);
}

void
gpio_low_id(gpio_t *g, size_t id)
{
    gpioWrite(g->table[id].gpio, LOW_VALUE);
}

void
gpio_high_id(gpio_t *g, size_t id)
{
    gpioWrite(g->table[id].gpio, HIGH_VALUE);
}

bool
gpio_get(gpio_t *g, const char *name)
{
    size_t id;

    if (! find_id(g, name, &id)) return false;
    return gpioRead(g->table[id].gpio);
}

bool
gpio_get_id(gpio_t *g, unsigned id)
{
    return gpioRead(g->table[id].gpio);
}

void
gpio_destroy(gpio_t *g)
{
     free(g);
}
