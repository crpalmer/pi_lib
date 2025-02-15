#ifndef __ST7735S__
#define __ST7735S__

#include "display.h"
#include "io.h"
#include "spi.h"

class ST7735S : public Display {
public:
    ST7735S(SPI *spi, Output *reset, Output *backlight);
    ~ST7735S() { }

    Canvas *create_canvas() override;
    void set_brightness(double pct) override;

protected:
    void draw(int x0, int y0, int w, uint8_t *data);
    friend class ST7735S_Canvas;

private:
    SPI *spi;
    Output *RST, *BL;

    void write_reg(unsigned char reg);
    void write_byte(unsigned char byte);

    void reset();
    void init_reg();
    void init_scan_direction();
    void set_window(unsigned char x, unsigned char y, unsigned char x_end, unsigned char y_end);
};

#endif
