#ifndef __IL9341_H__
#define __IL9341_H__

#include "display.h"
#include "io.h"
#include "spi.h"

class IL9341 : public Display {
public:
    IL9341(SPI *spi, Output *reset, Output *backlight, int width = 320, int height = 240);
    void reset();

    Canvas *create_canvas() override;
    void set_brightness(double pct) override;
    void paint(Canvas *canvas) override;

private:
    SPI *spi;
    Output *reset_pin;
    Output *backlight;
    const int width, height;
};

#endif
