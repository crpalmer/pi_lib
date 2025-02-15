#ifndef __IMAGE_PNG_H__
#define __IMAGE_PNG_H__

#include "image.h"
#include "pngle.h"

class ImagePNG : public Image {
public:
    ImagePNG(const char *fname);
    ~ImagePNG();

    bool get_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) override;

public:
    void on_draw(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4]);

private:
    pngle_t *pngle;
    uint8_t *data;
};

#endif
