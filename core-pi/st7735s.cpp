#include "pi.h"
#include "consoles.h"
#ifndef PLATFORM_linux
#include <pigpio.h>
#else
static inline int spiWrite(int spi, void *ptr, int n) { return 0; }
static inline int spiOpen(int spi, int speed, int flags) { return 0; }
#endif

#include "st7735s.h"

#define SPI 0

void ST7735S::write_reg(unsigned char reg)
{
    DC->set(0);
    spiWrite(SPI, (char *) &reg, 1);
}

void ST7735S::write_byte(unsigned char byte)
{
    DC->set(1);
    spiWrite(SPI, (char *) &byte, 1);
}

#if 0
static void write_word(unsigned short word)
{
    unsigned char data[2];

    DC->set(1);
    data[0] = (word >> 8);
    data[1] = (word & 0xff);
    spiWrite(SPI, (char *) data, 2);
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

ST7735S::ST7735S()
{
    RST = new GPOutput(27);
    DC = new GPOutput(22);
    BL = new GPOutput(17);

    if (spiOpen(SPI, 20*1000*1000, 0) < 0) {
	consoles_fatal_printf("Failed to open SPI!\n");
    }

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

void ST7735S::paint(Canvas *generic_c)
{
    ST7735S_Canvas *c = (ST7735S_Canvas *) generic_c;
    int w = c->get_width();
    int h = c->get_height();
    int bpp = c->get_bpp();

    set_window(0, 0, w-1, h-1);
    DC->set(1);
    for (int row = h-1; row >= 0; row--) {
       spiWrite(SPI, (char *) c->get_raw(0, row), w * bpp);
    }
}
