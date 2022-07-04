#ifndef __ST7735S__
#define __ST7735S__

#ifdef __cplusplus
extern "C" {
#endif

void st7735s_init();
void st7735s_paint(unsigned char *rgb_buffer);
void st7735s_set_brightness(double pct);

static inline int st7735s_get_width() { return 160; }
static inline int st7735s_get_height() { return 128; }
static inline int st7735s_get_bytes_per_pixel() { return 2; }

static inline void st7735s_pixel(unsigned char *pixel, unsigned char r, unsigned char g, unsigned char b)
{
    pixel[0] = (r & 0xf8) | (g >> 5);
    pixel[1] = ((g & 0xfc) << 5) | (b >> 3);
}

#ifdef __cplusplus
};
#endif

#endif
