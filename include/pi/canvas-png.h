#ifndef __CANVAS_PNG_H__
#define __CANVAS_PNG_H__

#include "canvas.h"
#include <png.h>

class CanvasPNG : public Canvas {
public:
    CanvasPNG(const char *fname);

    ~CanvasPNG() { fatal_free(raw); }

    int is_valid() { return !had_error; }

    RGB24 get_pixel(int x, int y);
    void set_pixel(int x, int y, Byte r, Byte g, Byte b);

private:
    png_bytep *data;
    unsigned char *raw;
    int bits;
    int had_error;
    int bytes_per_pixel;
};

#endif
