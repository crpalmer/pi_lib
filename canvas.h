#ifndef __CANVAS_H__
#define __CANVAS_H__

typedef unsigned char Byte;
typedef unsigned int RGB32;

#define WHITE (0xffffff)

class Canvas {
public:
     RGB32 rgb32(Byte r, Byte g, Byte b) {
	return (r << 16) | (g << 8) | b;
     }

     virtual RGB32 get_pixel(int x, int y) = 0;

     virtual void set_pixel(int x, int y, Byte r, Byte g, Byte b) = 0;

     void set_pixel(int x, int y, RGB32 rgb) {
	set_pixel(x, y, (rgb>>16) & 0xff, (rgb>>8) & 0xff, rgb&0xff);
     }

     virtual void fill(Byte r, Byte g, Byte b);

     void blank() { fill(0, 0, 0); }

     void up_down_line(int x, int y, int len, int lw, RGB32 color);

     void left_right_line(int x, int y, int len, int lw, RGB32 color);

     int get_width() { return w; }

     int get_height() { return h; }

     void nine_segment(int digit, int x, int y, int w, int h, RGB32 color = WHITE);

     void nine_segment_1(int digit, RGB32 color = WHITE) {
	int d = (w > h) ? h : w;
	nine_segment(digit, 0.05*w, 0.05*h, d*0.9, d*0.9, color);
     }

     void nine_segment_2(int digits, RGB32 color = WHITE) {
	nine_segment(digits / 10, 0.05*w, 0.05*h, w*0.4, h*0.9, color);
	nine_segment(digits % 10, 0.55*w, 0.05*h, w*0.4, h*0.9);
     }

protected:
     int w, h;
};

#endif
