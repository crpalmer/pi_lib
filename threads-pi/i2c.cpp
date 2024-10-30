#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "pi.h"
#include "pi-threads.h"
#include "i2c.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

const int MAX_BUS = 10;
const int MAX_FD = 4096;

static PiMutex *mutex[MAX_BUS];
static int fd_bus[MAX_FD];

static void i2c_lock(int fd) {
    assert(fd < MAX_FD);
    int bus = fd_bus[fd];
    assert(bus >= 0 && bus < MAX_BUS);
    mutex[bus]->lock();
}

static void i2c_unlock(int fd) {
    assert(fd < MAX_FD);
    int bus = fd_bus[fd];
    assert(bus >= 0 && bus < MAX_BUS);
    mutex[bus]->unlock();
}

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

    if (bus >= MAX_BUS) return -1;

    if (! mutex[bus]) mutex[bus] = new PiMutex();

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
