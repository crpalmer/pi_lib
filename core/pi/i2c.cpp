#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pi.h"
#include "core-lock.h"
#include "i2c.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

const int MAX_FD = 4096;
const int MAX_BUS = 10;

static int fd_bus[MAX_FD];
static core_lock_t lock[MAX_BUS];

static void i2c_lock(int fd) {
    assert(fd < MAX_FD);
    int bus = fd_bus[fd];
    assert(bus >= 0);
    core_lock_lock(lock[bus]);
}

static void i2c_unlock(int fd) {
    assert(fd < MAX_FD);
    int bus = fd_bus[fd];
    assert(bus >= 0);
    core_lock_lock(lock[bus]);
}

void i2c_init_bus(int bus, int sda, int scl, int speed)
{
    if (speed != 100*1000) {
	fprintf(stderr, "Warning: Ignoring bus speed request.\n");
    }
    lock[bus] = core_lock_create();
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

    if (fd >= MAX_FD) {
	close(fd);
	return -1;
    }

    fd_bus[fd] = bus;
    return fd;
}

void i2c_close(int fd)
{
    close(fd);
}

int i2c_read(int fd, unsigned char reg, void *data, int n_bytes)
{
    i2c_lock(fd);
    if (write(fd, &reg, 1) != 1) return -1;
    int ret = read(fd, data, n_bytes);
    i2c_unlock(fd);

    return ret;
}

int i2c_write(int fd, unsigned char reg, const void *data, int n_bytes)
{
    uint8_t buf[n_bytes+1];
    buf[0] = reg;
    memcpy(&buf[1], data, n_bytes);

    i2c_lock(fd);
    int ret = write(fd, buf, n_bytes+1);
    i2c_unlock(fd);

    return ret;
}

bool i2c_exists(int bus, int addr) {
    int fd = i2c_open(bus, addr);
    if (fd < 0) return false;
    uint8_t dummy;
    bool ret = read(fd, &dummy, 1);
    i2c_close(fd);
    return ret;
}
