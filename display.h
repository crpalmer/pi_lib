#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "canvas.h"

class Display {
public:
    virtual Canvas *create_canvas();
    virtual void set_brightness(double pct);
    virtual void paint(Canvas *canvas);
};

#endif
