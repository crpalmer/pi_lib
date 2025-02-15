#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "image.h"

static inline uint32_t RGB24_of(uint8_t r, uint8_t g, uint8_t b) { return (r << 16) | (g << 8) | b; }
static inline uint16_t RGB16_of(uint8_t r, uint8_t g, uint8_t b) { return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3); }

#define COLOR_WHITE 0xff, 0xff, 0xff
#define COLOR_BLACK 0, 0, 0
#define COLOR_RED 0xff, 0, 0
#define COLOR_GREEN 0, 0xff, 0
#define COLOR_BLUE 0, 0, 0xff

class CanvasData {
public:
     virtual void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

     virtual void flush(void) {}
};

class Canvas {
public:
     Canvas(int w, int h) : w(w), h(h) {}
     Canvas() : Canvas(0, 0) {}

     virtual void flush() {}

     virtual void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) = 0;

     virtual void set_pixels(int x0, int y0, int w, int h, uint8_t *rgb) {
	for (int x = x0; x < x0 + w; x++) {
	    for (int y = y0; y < y0 + h; y++) {
		set_pixel(x, y, rgb[0], rgb[1], rgb[2]);
		rgb += 3;
	    }
	}
     }

     virtual void fill(uint8_t r, uint8_t g, uint8_t b, int x0 = 0, int y0 = 0, int xw = 0, int yh = 0);

public:
     int get_width() { return w; }
     int get_height() { return h; }

     void blank(int x0 = 0, int y0 = 0, int xw = 0, int yh = 0) { fill(0, 0, 0, x0, y0, xw, yh); }

     void up_down_line(int x, int y, int len, int lw, uint8_t r, uint8_t g, uint8_t b);
     void left_right_line(int x, int y, int len, int lw, uint8_t r, uint8_t g, uint8_t b);

     void nine_segment(int digit, int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
     void nine_segment_1(int digit, uint8_t r, uint8_t g, uint8_t b) {
	int d = (w > h) ? h : w;
	nine_segment(digit, 0.05*w, 0.05*h, d*0.9, d*0.9, r, g, b);
     }

     void nine_segment_2(int digits, uint8_t r, uint8_t g, uint8_t b) {
	nine_segment(digits / 10, 0.05*w, 0.05*h, w*0.4, h*0.9, r, g, b);
	nine_segment(digits % 10, 0.55*w, 0.05*h, w*0.4, h*0.9, r, g, b);
     }

     void import(Image *image, int x = 0, int y = 0, int w = -1, int h = -1);

protected:
     int w, h;
};

#endif
