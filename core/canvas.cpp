#include <stdio.h>
#include "pi.h"
#include "canvas.h"

static int
line_width(int d)
{
    int lw = d * 0.1;
    return lw > 0 ? lw : 1;
}

static int digits[10][9] = { // top, mid, bottom, left 1/2, mid 1/2 right 1/2
    1, 0, 1, 1, 1, 0, 0, 1, 1,	// 0
    0, 0, 0, 0, 0, 1, 1, 0, 0,	// 1
    1, 1, 1, 0, 1, 0, 0, 1, 0,	// 2
    1, 1, 1, 0, 0, 0, 0, 1, 1,	// 3
    0, 1, 0, 1, 0, 0, 0, 1, 1,	// 4
    1, 1, 1, 1, 0, 0, 0, 0, 1, 	// 5
    1, 1, 1, 1, 1, 0, 0, 0, 1,	// 6
    1, 0, 0, 0, 0, 0, 0, 1, 1,	// 7
    1, 1, 1, 1, 1, 0, 0, 1, 1,	// 8
    1, 1, 0, 1, 0, 0, 0, 1, 1,	// 9
};

void Canvas::fill(uint8_t r, uint8_t g, uint8_t b, int x0, int y0, int xw, int yh) {
    if (xw == 0 || x0 + xw > w) xw = w - x0;
    if (yh == 0 || y0 + yh > h) yh = h - y0;

    for (int y = 0; y < yh; y++) {
	for (int x = 0; x < xw; x++) {
	    set_pixel(x+x0, y+y0, r, g, b);
	}
    }
}

void Canvas::up_down_line(int x, int y, int len, int lw, uint8_t r, uint8_t g, uint8_t b) {
    for (int draw_y = y; draw_y < y + len; draw_y++) {
	for (int draw_x = x; draw_x < x + lw; draw_x++) {
	   set_pixel(draw_x, draw_y, r, g, b);
	}
    }
}

void Canvas::left_right_line(int x, int y, int len, int lw, uint8_t r, uint8_t g, uint8_t b) {
    for (int draw_y = y; draw_y < y + lw; draw_y++) {
	for (int draw_x = x; draw_x < x + len; draw_x++) {
	   set_pixel(draw_x, draw_y, r, g, b);
	}
    }
}

void Canvas::nine_segment(int digit, int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    int lw;

    if (w > h) lw = line_width(h);
    else lw = line_width(w);

    if (digits[digit][0]) left_right_line(x+lw*2, y,          w - lw*4, lw, r, g, b);
    if (digits[digit][1]) left_right_line(x+lw*2, y+h/2-lw/2, w - lw*4, lw, r, g, b);
    if (digits[digit][2]) left_right_line(x+lw*2, y+h - lw,   w - lw*4, lw, r, g, b);

    if (digits[digit][3]) up_down_line(x,          y+lw,     h/2-lw*2, lw, r, g, b);
    if (digits[digit][4]) up_down_line(x,          y+h/2+lw, h/2-lw*2, lw, r, g, b);
 
    if (digits[digit][5]) up_down_line(x+(w-lw)/2, y+lw,     h/2-lw*2, lw, r, g, b);
    if (digits[digit][6]) up_down_line(x+(w-lw)/2, y+h/2+lw, h/2-lw*2, lw, r, g, b);

    if (digits[digit][7]) up_down_line(x+w-lw,     y+lw,     h/2-lw*2, lw, r, g, b);
    if (digits[digit][8]) up_down_line(x+w-lw,     y+h/2+lw, h/2-lw*2, lw, r, g, b);
}

void Canvas::import(Image *image, int x0, int y0, int w, int h)
{
    if (w <= 0 || w > x0+this->w) w = this->w - x0;
    if (h <= 0 || h > y0+this->h) h = this->h - y0;

    int o_w = image->get_width();
    int o_h = image->get_height();

    double scale_x = ((double) o_w) / w;
    double scale_y = ((double) o_h) / h;

    for (int y = y0; y < y0+h; y++) {
	for (int x = x0; x < x0+w; x++) {
	    uint8_t r, g, b;
	    if (image->get_pixel(x*scale_x, y*scale_y, &r, &g, &b)) {
		set_pixel(x, y, r, g, b);
	    }
 	}
    }
}
