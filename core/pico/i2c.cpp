#include "pi.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <memory.h>
#include "i2c.h"
#include "mem.h"

#define MAX_I2C  32

typedef struct {
    uint8_t bus;
    uint8_t addr;
} i2c_data_t;

static i2c_data_t i2c_data[MAX_I2C];

static inline i2c_inst_t *i2c_inst(int bus) { return bus ? i2c1 : i2c0; }

void i2c_init_bus(int bus, int sda, int scl, int speed)
{
    i2c_init(i2c_inst(bus), speed);
    gpio_set_function(sda, GPIO_FUNC_I2C);
    gpio_set_function(scl, GPIO_FUNC_I2C);
    gpio_pull_up(sda);
    gpio_pull_up(scl);
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

int i2c_read(int fd, uint8_t reg, void *data, size_t n_bytes)
{
    i2c_data_t *i2c = &i2c_data[fd];

    i2c_write_blocking(i2c_inst(i2c->bus), i2c->addr, &reg, 1, false);
    return i2c_read_blocking(i2c_inst(i2c->bus), i2c->addr, (uint8_t*) data, n_bytes, false);
}

static int i2c_write_internal(int fd, int reg, int reg_bytes, const void *data, size_t n_bytes) {
    i2c_data_t *i2c = &i2c_data[fd];
    uint8_t stack_buf[32];
    uint8_t *alloc_buf = NULL;
    uint8_t *msg;

    if (reg_bytes + n_bytes <= sizeof(stack_buf)) msg = stack_buf;
    else msg = alloc_buf = (uint8_t *) fatal_malloc(reg_bytes + n_bytes);

    for (int i = 0; i < reg_bytes; i++) {
	msg[i] = (uint8_t) ((reg >> 8 * (reg_bytes - i - 1)) & 0xff);
    }

    memcpy(&msg[reg_bytes], data, n_bytes);
    int ret = i2c_write_blocking(i2c_inst(i2c->bus), i2c->addr, msg, n_bytes+reg_bytes, false);
    if (alloc_buf) fatal_free(alloc_buf);

    if ((size_t) ret == n_bytes + reg_bytes) return n_bytes;
    return ret;
}

int i2c_write(int fd, uint8_t reg, const void *data, size_t n_bytes)
{
    return i2c_write_internal(fd, reg, 1, data, n_bytes);
}

int i2c_read_16(int fd, uint16_t reg, void *data, size_t n_bytes)
{
    i2c_data_t *i2c = &i2c_data[fd];
    uint8_t reg_bytes[2] = { (uint8_t) ((reg >> 8) & 0xff), (uint8_t) (reg & 0xff) };

    if (i2c_write_blocking(i2c_inst(i2c->bus), i2c->addr, reg_bytes, 2, false) != 2) return -1;
    return i2c_read_blocking(i2c_inst(i2c->bus), i2c->addr, (uint8_t*) data, n_bytes, false);
}

int i2c_write_16(int fd, uint16_t reg, const void *data, size_t n_bytes)
{
#if 1
    return i2c_write_internal(fd, reg, 2, data, n_bytes);
#else
    uint8_t msg[n_bytes+2];
    i2c_data_t *i2c = &i2c_data[fd];

    msg[0] = (uint8_t) ((reg >> 8) & 0xff);
    msg[1] = (uint8_t) (reg & 0xff);

    for (size_t i = 0; i < n_bytes; i++) {
	msg[i+2] = ((const uint8_t *) data)[i];
    }
    return i2c_write_blocking(i2c_inst(i2c->bus), i2c->addr, msg, n_bytes+2, false) - 2;
#endif
}

bool i2c_exists(int bus, int addr) {
    uint8_t dummy;
    return i2c_read_blocking(i2c_inst(bus), addr, &dummy, 1, false) == 1;
}
