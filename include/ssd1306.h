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

    Canvas *create_canvas();
    void set_brightness(double pct);
    void paint(Canvas *canvas);

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
