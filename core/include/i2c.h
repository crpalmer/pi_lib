#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>

/* init_bus and config_gpios are no-ops on the PI (only for the pico) */
void i2c_init_bus(int bus = 1, int sda = 2, int scl = 3, int bus_speed = 100*1000);

int i2c_open(int bus, int addr);
void i2c_close(int fd);
int i2c_read(int fd, uint8_t reg, void *data, int n_bytes);
int i2c_write(int fd, uint8_t reg, const void *data, int n_bytes);
bool i2c_exists(int bus, int addr);

static inline int i2c_read_byte(int fd, uint8_t reg, uint8_t *data)
{
    return i2c_read(fd, reg, data, 1);
}

static inline int i2c_write_byte(int fd, uint8_t reg, uint8_t data)
{
    return i2c_write(fd, reg, &data, 1);
}

static inline int i2c_read_word(int fd, uint8_t reg, uint16_t *data)
{
    return i2c_read(fd, reg, data, 2);
}

static inline int i2c_write_word(int fd, uint8_t reg, uint16_t data)
{
    return i2c_write(fd, reg, &data, 2);
}

#if PLATFORM_pi
extern void (*i2c_lock_fn)(int bus);
extern void (*i2c_unlock_fn)(int bus);
#endif

#endif
