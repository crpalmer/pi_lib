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

#define HIGH_VALUE 0
#define LOW_VALUE 1

struct gpioS {
     gpio_table_t *table;
     char        **value_fname;
     int           n_table;
};

static void
set_gpio_id(gpio_t *g, size_t i, int value)
{
    fatal_echo(g->value_fname[i], "%d\n", value);
}

static bool
get_gpio_id(gpio_t *g, size_t i)
{
    FILE *f = fopen(g->value_fname[i], "r");
    int value;

    if (! f) {
	perror(g->value_fname[i]);
	exit(1);
    }

    if (fscanf(f, "%d", &value) != 1) {
	fprintf(stderr, "Failed to read value from %s\n", g->value_fname[i]);
	exit(1);
    }

    fclose(f);

    return value != 0;
}
    
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

	if (g->table[i].initially_high == GPIO_IS_INPUT) {
	    fatal_echo(direction, "in\n");
	} else {
	    fatal_echo(direction, "out\n");
	    set_gpio_id(g, i, g->table[i].initially_high ? LOW_VALUE : HIGH_VALUE);
	}
	free(direction);
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
    set_gpio_id(g, id, value);
    return 0;
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
     set_gpio_id(g, id, LOW_VALUE);
}

void
gpio_high_id(gpio_t *g, size_t id)
{
     set_gpio_id(g, id, HIGH_VALUE);
}

bool
gpio_get(gpio_t *g, const char *name)
{
    size_t id;

    if (! find_id(g, name, &id)) return false;
    return gpio_get_id(g, id);
}

bool
gpio_get_id(gpio_t *g, unsigned id)
{
    return get_gpio_id(g, id);
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
