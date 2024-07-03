#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MAX_I2C  32

typedef struct {
    unsigned char bus;
    unsigned char addr;
} i2c_data_t;

static i2c_data_t i2c_data[MAX_I2C];

static inline i2c_inst_t *i2c_inst(int bus) { return bus ? i2c1 : i2c0; }

void i2c_init_bus(int bus, int sda, int scl, int speed)
{
    i2c_init(i2c_inst(bus), speed);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
}

int i2c_open(int bus, int addr)
{
    int fd;

    for (fd = 0; fd < MAX_I2C && i2c_data[fd].addr != 0; fd++) {}
    if (fd >= MAX_I2C) return -1;

    i2c_data[fd].bus = bus;
    i2c_data[fd].addr = addr;

    return fd;
}

void i2c_close(int fd)
{
    i2c_data[fd].addr = 0;
}

int i2c_read(int fd, unsigned char reg, void *data, int n_bytes)
{
    i2c_data_t *i2c = &i2c_data[fd];

    i2c_write_blocking(i2c_inst(i2c->bus), i2c->addr, &reg, 1, false);
    return i2c_read_blocking(i2c_inst(i2c->bus), i2c->addr, (uint8_t*) data, n_bytes, false);
}

int i2c_write(int fd, unsigned char reg, const void *data, int n_bytes)
{
    unsigned char msg[n_bytes+1];
    i2c_data_t *i2c = &i2c_data[fd];

    msg[0] = reg;
    for (int i = 0; i < n_bytes; i++) {
	msg[i+1] = ((const unsigned char *) data)[i];
    }
    return i2c_write_blocking(i2c_inst(i2c->bus), i2c->addr, msg, n_bytes+1, false);
}
