#include <stdio.h>
#include <stdlib.h>
#include "externals/PIGPIO/pigpio.h"
#include "gp-output.h"
#include "util.h"

#include "st7735s.h"

#define W() st7735s_get_width()
#define H() st7735s_get_height()
#define BPP() st7735s_get_bytes_per_pixel()

#define SPI 0

static GPOutput *RST;
static GPOutput *DC;
static GPOutput *BL;

static void write_reg(unsigned char reg)
{
    DC->set(0);
    spiWrite(SPI, (char *) &reg, 1);
}

static void write_byte(unsigned char byte)
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

static void
create_gpios()
{
    RST = new GPOutput(27);
    DC = new GPOutput(22);
    BL = new GPOutput(17);
    st7735s_set_brightness(1);
}

static void reset()
{
   RST->set(1);
   ms_sleep(100);
   RST->set(0);
   ms_sleep(100);
   RST->set(1);
   ms_sleep(100);
}

static void init_reg()
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

static void init_scan_direction()
{
    /* This set up to down, left to right */
    write_reg(0x36);
    write_byte(0x20);
    ms_sleep(200);
}

void st7735s_init()
{
    create_gpios();

    if (spiOpen(SPI, 20*1000*1000, 0) < 0) {
	fprintf(stderr, "Failed to open SPI!\n");
	exit(1);
    }

    reset();
    init_reg();
    init_scan_direction();
    write_reg(0x11);   // sleep out ?
    ms_sleep(120);
    write_reg(0x29);   // turn on the lcd display
}

static void
set_window(unsigned char x, unsigned char y, unsigned char x_end, unsigned char y_end)
{
#define X_OFFSET 1
#define Y_OFFSET 2

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

void st7735s_set_brightness(double brightness)
{
    BL->pwm(brightness);
}

void st7735s_paint(unsigned char *rgb_buffer)
{
    set_window(0, 0, W(), H());
    DC->set(1);
    for (int row = 0; row < H(); row++) {
       spiWrite(SPI, (char *) &rgb_buffer[row * W() * BPP()], W() * BPP());
    }
}
