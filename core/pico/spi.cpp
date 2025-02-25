#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "core-lock.h"
#include "io.h"
#include "gp-output.h"
#include "spi.h"

#define MAX_BUS 2
static core_lock_t lock[MAX_BUS];

static inline spi_inst_t *spi_inst_of(int bus) { return bus ? spi1 : spi0; }

void spi_init_bus(int bus, int sclk, int miso, int mosi, int speed) {
    spi_init(spi_inst_of(bus), speed);
    gpio_set_function(sclk, GPIO_FUNC_SPI);
    if (miso >= 0) gpio_set_function(miso, GPIO_FUNC_SPI);
    if (mosi >= 0) gpio_set_function(mosi, GPIO_FUNC_SPI);
    lock[bus] = core_lock_create();
}

void spi_lock_bus(int bus) {
    core_lock_lock(lock[bus]);
}

void spi_unlock_bus(int bus) {
    core_lock_unlock(lock[bus]);
}

SPI::SPI(int bus, int cs_pin, Output *dc) : bus(bus), dc(dc) {
    cs = new GPOutput(cs_pin);
}

int SPI::write(const uint8_t *bytes, const int n) {
    cs->set(0);
    spi_set_format(spi_inst_of(bus), 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    int res = spi_write_blocking(spi_inst_of(bus), bytes, n);
    cs->set(1);
    return res;
}

int SPI::write16(const uint16_t *words, const int n) {
    cs->set(0);
    spi_set_format(spi_inst_of(bus), 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    int res = spi_write16_blocking(spi_inst_of(bus), words, n);
    cs->set(1);
    return res;
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

int SPI::write_cmd(uint8_t cmd, const uint8_t *data, const int n) {
    assert(dc);
    dc->set(0);

    int ret = 0;

    if (write(&cmd, 1) != 1) ret = -1;
    if (ret == 0 && data && n && write_data(data, n) != n) ret = -1;

    return ret;
}
