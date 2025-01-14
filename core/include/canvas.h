#ifndef __CANVAS_H__
#define __CANVAS_H__

typedef unsigned char Byte;
typedef uint32_t RGB24;
typedef uint16_t RGB16;

#define WHITE (0xffffff)
#define RED   (0xff0000)
#define GREEN (0x00ff00)
#define BLUE  (0x0000ff)
#define BLACK (0x000000)

static inline RGB24 RGB24_of(uint8_t r, uint8_t g, uint8_t b) { return (r << 16) | (g << 8) | b; }
static inline RGB16 RGB16_of(uint8_t r, uint8_t g, uint8_t b) { return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3); }

static inline uint8_t RGB24_r(RGB24 rgb) { return (rgb >> 16) & 0xff; }
static inline uint8_t RGB24_g(RGB24 rgb) { return (rgb >> 8) & 0xff; }
static inline uint8_t RGB24_b(RGB24 rgb) { return rgb & 0xff; }

static inline uint8_t RGB16_r(RGB16 rgb) { uint8_t r = ((rgb >> 11) & 0x1f); return (r << 3) | (r >> 2); }
static inline uint8_t RGB16_g(RGB16 rgb) { uint8_t g = ((rgb >> 5) & 0x3f); return (g << 2) | (g >> 4); }
static inline uint8_t RGB16_b(RGB16 rgb) { uint8_t b = (rgb & 0x1f); return (b << 3) | (b >> 2); }

static inline RGB16 RGB24_to_RGB16(RGB24 rgb) {
    return RGB16_of(RGB24_r(rgb), RGB24_g(rgb), RGB24_b(rgb));
}

static inline RGB24 RGB16_to_RGB24(RGB16 rgb) {
    return RGB24_of(RGB16_r(rgb), RGB16_g(rgb), RGB16_b(rgb));
}

class Canvas {
public:
     int get_width() { return w; }

     int get_height() { return h; }

     int get_bpp() { return bpp; }

     virtual RGB24 get_pixel(int x, int y) { return RGB16_to_RGB24(get_pixel16(x, y)); }
     virtual RGB16 get_pixel16(int x, int y) { return RGB24_to_RGB16(get_pixel(x, y)); }

     virtual void set_pixel(int x, int y, Byte r, Byte g, Byte b) = 0;

     void set_pixel24(int x, int y, RGB24 rgb) {
	set_pixel(x, y, RGB24_r(rgb), RGB24_g(rgb), RGB24_b(rgb));
     }

     void set_pixel16(int x, int y, RGB16 rgb) {
	set_pixel(x, y, RGB16_r(rgb), RGB16_g(rgb), RGB16_b(rgb));
     }

     virtual void fill(Byte r, Byte g, Byte b, int x0 = 0, int y0 = 0, int xw = 0, int yh = 0);

     void fill24(RGB24 rgb, int x0 = 0, int y0 = 0, int xw = 0, int yh = 0) {
	fill(RGB24_r(rgb), RGB24_g(rgb), RGB24_b(rgb), x0, y0, xw, yh);
     }

     void fill16(RGB16 rgb, int x0 = 0, int y0 = 0, int xw = 0, int yh = 0) {
	fill(RGB16_r(rgb), RGB16_g(rgb), RGB16_b(rgb), x0, y0, xw, yh);
     }

     void blank(int x0 = 0, int y0 = 0, int xw = 0, int yh = 0)
     { fill(0, 0, 0, x0, y0, xw, yh); }

     void up_down_line(int x, int y, int len, int lw, RGB24 color);

     void left_right_line(int x, int y, int len, int lw, RGB24 color);

     void nine_segment(int digit, int x, int y, int w, int h, RGB24 color = WHITE);

     void nine_segment_1(int digit, RGB24 color = WHITE) {
	int d = (w > h) ? h : w;
	nine_segment(digit, 0.05*w, 0.05*h, d*0.9, d*0.9, color);
     }

     void nine_segment_2(int digits, RGB24 color = WHITE) {
	nine_segment(digits / 10, 0.05*w, 0.05*h, w*0.4, h*0.9, color);
	nine_segment(digits % 10, 0.55*w, 0.05*h, w*0.4, h*0.9);
     }

     void import(Canvas *other, int x = 0, int y = 0, int w = -1, int h = -1);

protected:
     int w, h, bpp;
};

#endif
