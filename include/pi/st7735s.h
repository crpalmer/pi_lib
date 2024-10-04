#ifndef __ST7735S__
#define __ST7735S__

#include <assert.h>

#include "canvas.h"
#include "display.h"
#include "gp-output.h"
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

class ST7735S_Canvas : public Canvas {
public:
    ST7735S_Canvas() {
	w = 160;
	h = 128;
	bpp = 2;
	raw = (unsigned char *) fatal_malloc(w*h*bpp);
    }

    ~ST7735S_Canvas() {
	fatal_free(raw);
    }

    int get_width() { return w; }
    int get_height() { return h; }
    int get_bytes_per_pixel() { return bpp; }

    RGB24 get_pixel(int x, int y) {
	unsigned char *pixel = get_raw(x, y);
	Byte r = pixel[0] >> 3;
	Byte g = (pixel[0] & 0x07) << 3 | (pixel[1] >> 5);
	Byte b = pixel[1] >> 3;
	return RGB24_of(r, g, b);
    }

    void set_pixel(int x, int y, Byte r, Byte g, Byte b) {
	unsigned char *pixel = get_raw(x, y);
	pixel[0] = (r & 0xf8) | (g >> 5);
	pixel[1] = ((g & 0xfc) << 5) | (b >> 3);
    }

    unsigned char *get_raw(int x, int y) {
	assert(x >= 0 && x < w);
	assert(y >= 0 && y < h);
	return &raw[bpp*(y * w + x)];
    }

private:
    unsigned char *raw;

    friend class ST7735S;
};

class ST7735S : public Display {
public:
    ST7735S();
    ~ST7735S() { }

    Canvas *create_canvas() { return new ST7735S_Canvas(); }

    void set_brightness(double pct);

    void paint(Canvas *canvas);

private:
    GPOutput *RST, *DC, *BL;

    void write_reg(unsigned char reg);
    void write_byte(unsigned char byte);

    void reset();
    void init_reg();
    void init_scan_direction();
    void set_window(unsigned char x, unsigned char y, unsigned char x_end, unsigned char y_end);

};

#ifdef __cplusplus
};
#endif

#endif
