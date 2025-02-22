#ifndef __ST7796S__
#define __ST7796S__

#include "display.h"
#include "io.h"
#include "spi.h"

class ST7796S : public Display {
public:
    ST7796S(SPI *spi, Output *reset, Output *backlight);
    ~ST7796S() { }

    Canvas *create_canvas(bool prefer_unbuffered) override;
    void set_brightness(double pct) override;
    void draw(int x0, int y0, int x_end, int y_end, uint8_t *data) override;

private:
    SPI *spi;
    Output *RST, *BL;

    void write_reg(unsigned char reg);
    void write_byte(unsigned char byte);

    void reset();
    void init_reg();
    void init_scan_direction();
    void set_window(uint16_t x0, uint16_t y0, uint16_t x_end, uint16_t y_end);
};

#endif
