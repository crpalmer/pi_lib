#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>

/* init_bus and config_gpios are no-ops on the PI (only for the pico) */
int i2c_init_bus(int bus, int bus_speed = 100*1000);
void i2c_config_gpios(int sda, int scl);

int i2c_open(int bus, int addr);
void i2c_close(int fd);
int i2c_read(int fd, unsigned char reg, void *data, int n_bytes);
int i2c_write(int fd, unsigned char reg, const void *data, int n_bytes);

static inline int i2c_read_byte(int fd, unsigned char reg, unsigned char *data)
{
    return i2c_read(fd, reg, data, 1);
}

static inline int i2c_write_byte(int fd, unsigned char reg, unsigned char data)
{
    return i2c_write(fd, reg, &data, 1);
}

static inline int i2c_read_word(int fd, unsigned char reg, unsigned short *data)
{
    return i2c_read(fd, reg, data, 2);
}

static inline int i2c_write_word(int fd, unsigned char reg, unsigned short data)
{
    return i2c_write(fd, reg, &data, 2);
}

#endif
