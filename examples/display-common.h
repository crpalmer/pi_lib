#ifndef __DISPLAY_COMMON_H__
#define __DISPLAY_COMMON_H__

#include "i2c.h"
#include "il9341.h"
#include "spi.h"
#include "ssd1306.h"
#include "st7735s.h"
#include "st7796s.h"

typedef enum { USE_SSD1306, USE_IL9341, USE_ST7735S, USE_ST7796S } which_display_t;

static Display *create_display(which_display_t which_display) {
    if (which_display == USE_SSD1306) {
        i2c_init_bus(1, 2, 3, 400*1000);
        return new SSD1306(1);
    } else {
        spi_init_bus(1, 10, -1, 11, 10*1024*1024);

        Output *bl = new GPOutput(6);
        Output *reset = new GPOutput(7);
        Output *dc = new GPOutput(8);

        SPI *spi = new SPI(1, 9, dc);

	switch (which_display) {
        case USE_IL9341:  return new IL9341(spi, reset, bl);
        case USE_ST7735S: return new ST7735S(spi, reset, bl);
        case USE_ST7796S: return new ST7796S(spi, reset, bl);
	default: return NULL;
        }
    }
}

#endif
