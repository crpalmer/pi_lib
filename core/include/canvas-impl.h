#ifndef __CANVAS_IMPL_H__
#define __CANVAS_IMPL_H__

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

     virtual void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

protected:
     Display *display;
     int bytes_per_pixel;
     uint8_t *buffer;
};

#endif
