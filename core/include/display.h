#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "canvas.h"

class Display {
public:
    virtual Canvas *create_canvas(bool prefer_unbuffered = false) = 0;
    virtual void set_brightness(double pct) = 0;
    virtual void draw(int x, int y, int x_max, int y_max, uint8_t *data) = 0;
};

#endif
