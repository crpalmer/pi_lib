#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "canvas.h"

class Display {
public:
    virtual Canvas *create_canvas() = 0;
    virtual void set_brightness(double pct) = 0;
};

#endif
