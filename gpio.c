#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mem.h"
#include "file.h"
#include "string-utils.h"
#include "gpio.h"

#define GPIO_DIR_PREFIX "/sys/class/gpio/gpio"
#define EXPORT_FNAME "/sys/class/gpio/export"
#define UNEXPORT_FNAME "/sys/class/gpio/unexport"

struct gpioS {
     gpio_table_t *table;
     char        **value_fname;
     int           n_table;
};

gpio_t *
gpio_new(gpio_table_t *table, int n_table)
{
     int i;
     gpio_t *g = fatal_malloc(sizeof(*g));

     g->table = table;
     g->n_table = n_table;
     g->value_fname = fatal_malloc(sizeof(*g->value_fname) * g->n_table);

     for (i = 0; i < g->n_table; i++) {
	char *direction = maprintf("%s%d/direction", GPIO_DIR_PREFIX, g->table[i].gpio);
	g->value_fname[i] = maprintf("%s%d/value", GPIO_DIR_PREFIX, g->table[i].gpio);

	fatal_echo(EXPORT_FNAME, "%d\n", g->table[i].gpio);

	fatal_echo(direction, "out\n");
	fatal_echo(g->value_fname[i], "%d\n", g->table[i].initially_high ? 0 : 1);
	free(direction);
     }

     return g;
}

static int
set_gpio(gpio_t *g, const char *name, int value)
{
     int i;

     for (i = 0; i < g->n_table; i++) {
	if (strcmp(g->table[i].name, name) == 0) {
	     fatal_echo(g->value_fname[i], "%d\n", value);
	     return 0;
	}
    }
    return -1;
}


int
gpio_low(gpio_t *g, const char *name)
{
     return set_gpio(g, name, 0);
}

int
gpio_high(gpio_t *g, const char *name)
{
     return set_gpio(g, name, 1);
}

void
gpio_destroy(gpio_t *g)
{
     int i;

     for (i = 0; i < g->n_table; i++) {
	echo(UNEXPORT_FNAME, "%d\n", g->table[i].gpio);
	free(g->value_fname[i]);
     }

     free(g->value_fname);
     free(g);
}