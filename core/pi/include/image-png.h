#ifndef __IMAGE_PNG_H__
#define __IMAGE_PNG_H__

#include "image.h"
#include <png.h>

class ImagePNG : public Image {
public:
    ImagePNG(const char *fname);
    ~ImagePNG() { fatal_free(raw); } // TODO: memory leak

    int is_valid() { return !had_error; }
    bool get_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) override;

private:
    png_bytep *data;
    unsigned char *raw;
    int bits;
    int had_error;
    int bytes_per_pixel;
};

#endif
