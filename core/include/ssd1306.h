#ifndef __SSD1306__
#define __SSD1306__

#include <assert.h>
#include <string.h>
#include "canvas.h"
#include "display.h"
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif

class SSD1306 : public Display {
public:
    SSD1306(int bus = 0, int addr = 0x3c);
    ~SSD1306() { }

    Canvas *create_canvas(bool prefer_unbuffered);
    void set_brightness(double pct);

protected:
    void draw(int page, uint8_t *raw);
    friend class SSD1306_Canvas;

private:
    void write_cmd(uint8_t cmd);
    void write_cmd(uint8_t cmd, uint8_t v1);
    void write_cmd(uint8_t cmd, uint8_t v1, uint8_t v2);
    void write_data(uint8_t *data, int n_data);

    int fd;
};

#ifdef __cplusplus
};
#endif

#endif
