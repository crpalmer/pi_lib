#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "i2c.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

void i2c_init_bus(int bus, int sda, int scl, int speed)
{
    if (speed != 100*1000) {
	fprintf(stderr, "Warning: Ignoring bus speed request.\n");
    }
}

int i2c_open(int bus, int addr)
{
    char fname[128];
    int fd, err;

    sprintf(fname, "/dev/i2c-%d", bus);
    if ((fd = open(fname, O_RDWR)) < 0) {
	perror(fname);
	return fd;
    }

    if ((err = ioctl(fd, I2C_SLAVE, addr)) < 0) {
        perror("ioctl");
        return err;
    }

    return fd;
}

void i2c_close(int fd)
{
    close(fd);
}

int i2c_read(int fd, unsigned char reg, void *data, int n_bytes)
{
    return read(fd, data, n_bytes);
}

int i2c_write(int fd, unsigned char reg, const void *data, int n_bytes)
{
    uint8_t buf[n_bytes+1];
    buf[0] = reg;
    memcpy(&buf[1], data, n_bytes);
    return write(fd, buf, n_bytes+1);
}
