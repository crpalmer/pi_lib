#include "pi.h"
#include "consoles.h"
#include "spi.h"

#include "st7735s.h"
#include "canvas-impl.h"

#define WIDTH 160
#define HEIGHT 120
#define BYTES_PER_PIXEL 2

class ST7735S_BufferedCanvas : public BufferedCanvas {
public:
    ST7735S_BufferedCanvas(ST7735S *display) : BufferedCanvas(display, WIDTH, HEIGHT, BYTES_PER_PIXEL) {
    }

    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) override {
        uint16_t rgb = RGB16_of(r, g, b); 
	uint8_t bytes[2];
	bytes[0] = rgb >> 8;	
	bytes[1] = rgb & 0xff;
	set_pixel_raw(x, y, bytes);
    }
};

class ST7735S_UnbufferedCanvas : public UnbufferedCanvas {
public:
    ST7735S_UnbufferedCanvas(ST7735S *display) : UnbufferedCanvas(display, WIDTH, HEIGHT, BYTES_PER_PIXEL, 16) {
    }

    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) override {
        uint16_t rgb = RGB16_of(r, g, b); 
	uint8_t bytes[2];
	bytes[0] = rgb >> 8;	
	bytes[1] = rgb & 0xff;
	set_pixel_raw(x, y, bytes);
    }
};

Canvas *ST7735S::create_canvas(bool prefer_unbuffered) {
    if (prefer_unbuffered) return new ST7735S_UnbufferedCanvas(this);
    else return new ST7735S_BufferedCanvas(this);
}

void ST7735S::write_reg(unsigned char reg)
{
    spi->write_cmd(reg);
}

void ST7735S::write_byte(unsigned char byte)
{
    spi->write_data(&byte, 1);
}

#if 0
static void write_word(unsigned short word)
{
    unsigned char data[2];

    DC->set(1);
    data[0] = (word >> 8);
    data[1] = (word & 0xff);
    spi->write_data((char *) data, 2);
}
#endif

void ST7735S::reset()
{
   RST->set(1);
   ms_sleep(100);
   RST->set(0);
   ms_sleep(100);
   RST->set(1);
   ms_sleep(100);
}

void ST7735S::init_reg()
{
    //ST7735R Frame Rate
    write_reg(0xB1);
    write_byte(0x01);
    write_byte(0x2C);
    write_byte(0x2D);

    write_reg(0xB2);
    write_byte(0x01);
    write_byte(0x2C);
    write_byte(0x2D);

    write_reg(0xB3);
    write_byte(0x01);
    write_byte(0x2C);
    write_byte(0x2D);
    write_byte(0x01);
    write_byte(0x2C);
    write_byte(0x2D);

    write_reg(0xB4); //Column inversion
    write_byte(0x07);

    //ST7735R Power Sequence
    write_reg(0xC0);
    write_byte(0xA2);
    write_byte(0x02);
    write_byte(0x84);
    write_reg(0xC1);
    write_byte(0xC5);

    write_reg(0xC2);
    write_byte(0x0A);
    write_byte(0x00);

    write_reg(0xC3);
    write_byte(0x8A);
    write_byte(0x2A);
    write_reg(0xC4);
    write_byte(0x8A);
    write_byte(0xEE);

    write_reg(0xC5); //VCOM
    write_byte(0x0E);

    //ST7735R Gamma Sequence
    write_reg(0xe0);
    write_byte(0x0f);
    write_byte(0x1a);
    write_byte(0x0f);
    write_byte(0x18);
    write_byte(0x2f);
    write_byte(0x28);
    write_byte(0x20);
    write_byte(0x22);
    write_byte(0x1f);
    write_byte(0x1b);
    write_byte(0x23);
    write_byte(0x37);
    write_byte(0x00);
    write_byte(0x07);
    write_byte(0x02);
    write_byte(0x10);

    write_reg(0xe1);
    write_byte(0x0f);
    write_byte(0x1b);
    write_byte(0x0f);
    write_byte(0x17);
    write_byte(0x33);
    write_byte(0x2c);
    write_byte(0x29);
    write_byte(0x2e);
    write_byte(0x30);
    write_byte(0x30);
    write_byte(0x39);
    write_byte(0x3f);
    write_byte(0x00);
    write_byte(0x07);
    write_byte(0x03);
    write_byte(0x10);

    write_reg(0xF0); //Enable test command
    write_byte(0x01);

    write_reg(0xF6); //Disable ram power save mode
    write_byte(0x00);

    write_reg(0x3A); //65k mode
    write_byte(0x05);

}

void ST7735S::init_scan_direction()
{
    /* This set up to down, left to right */
    write_reg(0x36);
    write_byte(0x20);
    ms_sleep(200);
}

ST7735S::ST7735S(SPI *spi, Output *RST, Output *BL) : spi(spi), RST(RST), BL(BL) {
    set_brightness(0);

    reset();
    init_reg();
    init_scan_direction();
    write_reg(0x11);   // sleep out ?
    ms_sleep(120);
    write_reg(0x29);   // turn on the lcd display

    set_brightness(1);
}

void ST7735S::set_window(unsigned char x, unsigned char y, unsigned char x_end, unsigned char y_end)
{
#define X_OFFSET 1
#define Y_OFFSET 3

    write_reg(0x2a);
    write_byte(0);
    write_byte(x + X_OFFSET);
    write_byte(0);
    write_byte(x_end + X_OFFSET);

    write_reg(0x2b);
    write_byte(0);
    write_byte(y + Y_OFFSET);
    write_byte(0);
    write_byte(y_end + Y_OFFSET);

    write_reg(0x2c);
}

void ST7735S::set_brightness(double brightness)
{
    BL->pwm(brightness);
}

void ST7735S::draw(int x0, int y0, int x_max, int y_max, uint8_t *data) {
    set_window(x0, y0, x_max-1, y_max-1);
    spi->write_data(data, (x_max - x0 + 1) * (y_max - y0 + 1) * BYTES_PER_PIXEL);
}
