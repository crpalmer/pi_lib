#ifndef __CANVAS_H__
#define __CANVAS_H__

typedef unsigned char Byte;
typedef unsigned int RGB32;

class Canvas {
public:
     virtual RGB32 get_pixel(int x, int y) = 0;

     virtual void set_pixel(int x, int y, Byte r, Byte g, Byte b) = 0;

     void set_pixel(int x, int y, RGB32 rgb) {
	set_pixel(x, y, (rgb>>16) & 0xff, (rgb>>8) & 0xff, rgb&0xff);
     }

     void fill(Byte r, Byte g, Byte b);

     int get_width() { return w; }

     int get_height() { return h; }

protected:
     int w, h;
};

#endif
