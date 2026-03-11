#ifndef __FRAM_AT24C__
#define __FRAM_AT24C__

#include "fram.h"
#include "i2c.h"
#include "time-utils.h"

class FRAM_AT24C : public FRAM {
public:
    FRAM_AT24C(int bus, int sub_address = 0) {
	i2c_address = 0x50 + sub_address;
	if ((fd = i2c_open(bus, i2c_address)) < 0) {
	    printf("Failed to open i2c bus %d address 0x%02x\n", bus, i2c_address);
	    pi_abort();
	}
    }

    size_t get_capacity() override { return 256; }

    bool read(int address, void *data, size_t n_bytes) override {
	uint8_t *ptr = (uint8_t *) data;
	for (size_t start = 0; start < n_bytes; start += CHUNK) {
	    size_t n = n_bytes - start;
	    if (n > CHUNK) n = CHUNK;

	    ms_sleep(5);

	    int ret;
	    if ((ret = i2c_read(fd, address + start, ptr + start, CHUNK)) != CHUNK) {
		printf("0x%02x read @ %d failure: %d\n", i2c_address, (int) address + start, ret);
		return false;
	    }
	}
	return true;
    }

    bool write(int address, void *data, size_t n_bytes) override {
	uint8_t *ptr = (uint8_t *) data;
	for (size_t start = 0; start < n_bytes; start += CHUNK) {
	    size_t n = n_bytes - start;
	    if (n > CHUNK) n = CHUNK;

	    ms_sleep(5);

	    int ret;
	    if ((ret = i2c_write(fd, address + start, ptr + start, CHUNK)) != CHUNK+1) {
		printf("0x%02x write @ %d failure: %d\n", i2c_address, (int) start, ret);
		return false;
	    }
	}
	return true;
    }

private:
    int i2c_address;
    int fd;

    static const size_t CHUNK = 16;
};

#endif
