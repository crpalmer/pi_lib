#ifndef __FRAM_MB85C__
#define __FRAM_MB85C__

#include "fram.h"
#include "i2c.h"
#include "time-utils.h"

class FRAM_MB85C : public FRAM {
public:
    FRAM_MB85C(int bus, int sub_address = 0) {
	i2c_address = 0x50 + sub_address;
	if ((fd = i2c_open(bus, i2c_address)) < 0) {
	    printf("Failed to open i2c bus %d address 0x%02x\n", bus, i2c_address);
	    pi_abort();
	}
    }

    size_t get_capacity() override { return capacity; }

    bool read(int address, void *data, size_t n_bytes) override {
	return i2c_read_16(fd, address, data, n_bytes) == (int) n_bytes;
    }

    bool write(int address, const void *data, size_t n_bytes) override {
	return i2c_write_16(fd, address, data, n_bytes) == (int) n_bytes;
    }

private:
    int i2c_address;
    int fd;
    int capacity = 32*1024;
};

#endif
