#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "canvas.h"

class Display {
public:
    Canvas *create_canvas();
    void set_brightness(double pct);
    void paint(Canvas *canvas);
};

#endif
