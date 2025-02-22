#include "pi.h"
#include "consoles.h"
#include "spi.h"

#include "st7796s.h"
#include "canvas-impl.h"

#define WIDTH 480
#define HEIGHT 320
#define BYTES_PER_PIXEL 2
#define BUFFERED_ROWS 16

class ST7796S_BufferedCanvas : public BufferedCanvas {
public:
    ST7796S_BufferedCanvas(ST7796S *display) : BufferedCanvas(display, WIDTH, HEIGHT, BYTES_PER_PIXEL) {
    }

    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
	uint16_t rgb565 = RGB16_of(b, g, r);		// Always seems to BGR even though I request RGB
        uint8_t pixel[2];
        pixel[0] = rgb565 >> 8;
        pixel[1] = rgb565 & 0xff;
	set_pixel_raw(x, y, pixel);
    }
};

class ST7796S_UnbufferedCanvas : public UnbufferedCanvas {
public:
    ST7796S_UnbufferedCanvas(ST7796S *display) : UnbufferedCanvas(display, WIDTH, HEIGHT, BYTES_PER_PIXEL, BUFFERED_ROWS) {
    }

    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) override {
	uint16_t rgb565 = RGB16_of(b, g, r);		// Always seems to BGR even though I request RGB
        uint8_t pixel[2];
        pixel[0] = rgb565 >> 8;
        pixel[1] = rgb565 & 0xff;
	set_pixel_raw(x, y, pixel);
    }
};

Canvas *ST7796S::create_canvas(bool prefer_unbuffered) {
    if (prefer_unbuffered) return new ST7796S_UnbufferedCanvas(this);
    else return new ST7796S_BufferedCanvas(this);
}

void ST7796S::write_reg(unsigned char reg)
{
    spi->write_cmd(reg);
}

void ST7796S::write_byte(unsigned char byte)
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

void ST7796S::reset()
{
   RST->set(1);
   ms_sleep(100);
   RST->set(0);
   ms_sleep(100);
   RST->set(1);
   ms_sleep(100);
}

void ST7796S::init_reg()
{
    ms_sleep(120);

    write_reg(0x01); //Software reset
    ms_sleep(120);

    write_reg(0x11); //Sleep exit
    ms_sleep(120);

    write_reg(0xF0); //Command Set control
    write_byte(0xC3);    //Enable extension command 2 partI

    write_reg(0xF0); //Command Set control                                 
    write_byte(0x96);    //Enable extension command 2 partII

    write_reg(0x36); //Memory Data Access Control MX, MY, RGB mode
    write_byte(0x48);    //X-Mirror, Top-Left to right-Buttom, RGB  
	
    write_reg(0x3A); //Interface Pixel Format
    write_byte(0x55);    //Control interface color format set to 16

    write_reg(0xB4); //Column inversion
    write_byte(0x01);    //1-dot inversion

    write_reg(0xB6); //Display Function Control
    write_byte(0x80);    //Bypass
    write_byte(0x02);    //Source Output Scan from S1 to S960, Gate Output scan from G1 to G480, scan cycle=2
    write_byte(0x3B);    //LCD Drive Line=8*(59+1)

    write_reg(0xE8); //Display Output Ctrl Adjust
    write_byte(0x40);
    write_byte(0x8A);	
    write_byte(0x00);
    write_byte(0x00);
    write_byte(0x29);    //Source eqaulizing period time= 22.5 us
    write_byte(0x19);    //Timing for "Gate start"=25 (Tclk)
    write_byte(0xA5);    //Timing for "Gate End"=37 (Tclk), Gate driver EQ function ON
    write_byte(0x33);
	
    write_reg(0xC1); //Power control2
    write_byte(0x06);    //VAP(GVDD)=3.85+( vcom+vcom offset), VAN(GVCL)=-3.85+( vcom+vcom offset)

    write_reg(0xC2); //Power control 3
    write_byte(0xA7);    //Source driving current level=low, Gamma driving current level=High
	 
    write_reg(0xC5); //VCOM Control
    write_byte(0x18);    //VCOM=0.9

    ms_sleep(120);
	
    //ST7796 Gamma Sequence
    write_reg(0xE0); //Gamma"+"
    write_byte(0xF0);
    write_byte(0x09); 
    write_byte(0x0b);
    write_byte(0x06); 
    write_byte(0x04);
    write_byte(0x15); 
    write_byte(0x2F);
    write_byte(0x54); 
    write_byte(0x42);
    write_byte(0x3C); 
    write_byte(0x17);
    write_byte(0x14);
    write_byte(0x18); 
    write_byte(0x1B); 

    write_reg(0xE1); //Gamma"-"                                             
    write_byte(0xE0);
    write_byte(0x09); 
    write_byte(0x0B);
    write_byte(0x06); 
    write_byte(0x04);
    write_byte(0x03); 
    write_byte(0x2B);
    write_byte(0x43); 
    write_byte(0x42);
    write_byte(0x3B); 
    write_byte(0x16);
    write_byte(0x14);
    write_byte(0x17); 
    write_byte(0x1B);

    ms_sleep(120);
	
    write_reg(0xF0); //Command Set control
    write_byte(0x3C);    //Disable extension command 2 partI

    write_reg(0xF0); //Command Set control
    write_byte(0x69);    //Disable extension command 2 partII

    ms_sleep(120);

    write_reg(0x29); //Display on      
}

void ST7796S::init_scan_direction()
{
    /* This set up to down, left to right */
    write_reg(0x36);
    write_byte(0x20);
    ms_sleep(200);
}

ST7796S::ST7796S(SPI *spi, Output *RST, Output *BL) : spi(spi), RST(RST), BL(BL) {
    set_brightness(0);

    reset();
    init_reg();
    init_scan_direction();
    write_reg(0x11);   // sleep out ?
    ms_sleep(120);
    write_reg(0x29);   // turn on the lcd display

    set_brightness(1);
}

void ST7796S::set_window(uint16_t x, uint16_t y, uint16_t x_end, uint16_t y_end)
{
    write_reg(0x2a);
    write_byte(x >> 8);
    write_byte(x & 0xff);
    write_byte(x_end >> 8);
    write_byte(x_end & 0xff);

    write_reg(0x2b);
    write_byte(y >> 8);
    write_byte(y & 0xff);
    write_byte(y_end >> 8);
    write_byte(y_end & 0xff);

    write_reg(0x2c);
}

void ST7796S::set_brightness(double brightness)
{
    BL->pwm(brightness);
}

void ST7796S::draw(int x0, int y0, int x_end, int y_end, uint8_t *data) {
    set_window(x0, y0, x_end, y_end);
    spi->write_data(data, (x_end - x0 + 1) * (y_end - y0 + 1) * BYTES_PER_PIXEL);
}
