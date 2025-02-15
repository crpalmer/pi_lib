#ifndef __ST7735S__
#define __ST7735S__

#include <assert.h>

#include "canvas.h"
#include "display.h"
#include "gp-output.h"
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

class ST7735S : public Display {
public:
    ST7735S();
    ~ST7735S() { }

    Canvas *create_canvas() override;
    void set_brightness(double pct) override;

protected:
    void draw(int x0, int y0, int w, uint8_t *data);
    friend class ST7735S_Canvas;

private:
    GPOutput *RST, *DC, *BL;

    void write_reg(unsigned char reg);
    void write_byte(unsigned char byte);

    void reset();
    void init_reg();
    void init_scan_direction();
    void set_window(unsigned char x, unsigned char y, unsigned char x_end, unsigned char y_end);

};

#ifdef __cplusplus
};
#endif

#endif
