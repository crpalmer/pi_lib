#ifndef __SPI_H__
#define __SPI_H__

#include "io.h"

void spi_init_bus(int bus, int sclk, int miso = -1, int mosi = -1, int speed = 40*1000*1000);

class SPI {
public:
    SPI(int bus, int cs_pin, Output *dc = NULL);

    int write(const uint8_t *bytes, const int n);
    int write16(const uint16_t *words, const int n);
    int write_data(const uint8_t *bytes, const int n);
    int write_data16(const uint16_t *words, const int n);
    int write_cmd(uint8_t cmd);

private:
    int bus;
    Output *cs;
    Output *dc;
};

#endif
