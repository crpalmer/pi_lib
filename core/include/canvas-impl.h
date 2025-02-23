#ifndef __CANVAS_IMPL_H__
#define __CANVAS_IMPL_H__

#include "display.h"
#include "mem.h"

class BufferedCanvas : public Canvas {
public:
    BufferedCanvas(Display *display, int w, int h, int bytes_per_pixel) : Canvas(w, h), display(display), bytes_per_pixel(bytes_per_pixel) {
	buffer = (uint8_t *) fatal_malloc(w*h*bytes_per_pixel);
    }

    ~BufferedCanvas() {
	fatal_free(buffer);
    }

    void flush() override {
	display->draw(0, 0, w-1, h-1, buffer);
    }

    void set_pixel_raw(int x, int y, uint8_t *pixel) {
	uint8_t *pixels = &buffer[(x + y * w) * bytes_per_pixel];
	for (int i = 0; i < bytes_per_pixel; i++) {
	    pixels[i] = pixel[bytes_per_pixel-i-1];
	}
    }

    virtual void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) = 0;

protected:
     Display *display;
     int bytes_per_pixel;
     uint8_t *buffer;
};

class UnbufferedCanvas : public Canvas {
public:
    UnbufferedCanvas(Display *display, int w, int h, int bytes_per_pixel, int buffered_rows) : Canvas(w, h), display(display), bytes_per_pixel(bytes_per_pixel), buffered_rows(buffered_rows) {
	dirty = (uint8_t *) fatal_malloc(w*buffered_rows*bytes_per_pixel);
    }

    ~UnbufferedCanvas() {
	fatal_free(dirty);
    }

    void flush() override;

    void set_pixel_raw(int x, int y, uint8_t *pixel);
    virtual void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) = 0;

protected:
    Display *display;
    int bytes_per_pixel;
    int buffered_rows;
    uint8_t *dirty;
    int dirty_x = -1, dirty_first_x_max = -1, dirty_cur_x_max = -1;
    int dirty_y = -1, dirty_y_max = -1;

private:
    bool needs_to_be_flushed(int x, int y);
    uint8_t *dirty_at(int x, int y);
};

#endif
