#ifndef __IMAGE_H__
#define __IMAGE_H__

class Image {
public:
    Image(int w, int h) : w(w), h(h) {}
    Image() {}
    virtual ~Image() {}

    virtual int is_valid() { return true; }
    virtual bool get_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) = 0;

    int get_width() { return w; }
    int get_height() { return h; }

protected:
    int w;
    int h;
};

#endif
