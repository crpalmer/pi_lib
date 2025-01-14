#include "pi.h"
#include "pigpio.h"
#include "spi.h"

static int bus_speed[2] = { 10*1000*1000, 10*1000*1000 };

void spi_init_bus(int bus, int sclk, int miso, int mosi, int speed) {
    assert(bus == 0 || bus == 1);
    bus_speed[bus] = speed;
}

SPI::SPI(int bus, int cs_pin, Output *dc) : dc(dc) {
    assert(bus == 0 || bus == 1);
    bus = spiOpen(bus, bus_speed[bus], 0);
    assert(bus >= 0);
}

// TODO: Add a destructor

int SPI::write(const uint8_t *bytes, const int n) {
    return spiWrite(bus, (char*) bytes, n);
}

int SPI::write16(const uint16_t *words, const int n) {
    return spiWrite(bus, (char*) words, n*2);
}

int SPI::write_data(const uint8_t *bytes, const int n) {
    assert(dc);
    dc->set(1);
    return write(bytes, n);
}

int SPI::write_data16(const uint16_t *words, const int n) {
    assert(dc);
    dc->set(1);
    return write16(words, n);
}

int SPI::write_cmd(uint8_t cmd) {
    assert(dc);
    dc->set(0);
    return write(&cmd, 1);
}
