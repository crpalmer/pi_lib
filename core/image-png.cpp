#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mem.h"
#include "pi.h"

#include "image-png.h"
#include "pngle.h"

class OnDraw {
public:
    virtual ~OnDraw() {}
    virtual void on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4]) = 0;
};

class ImagePNG : public Image {
public:
    ~ImagePNG() {
	fatal_free(data);
    }

    bool get_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) {
	uint8_t *rgb = &data[(x + y*w) * 4];

	if (rgb[3] == 0) return false;

	*r = rgb[0];
	*g = rgb[1];
	*b = rgb[2];

	return true;
    }

    void on_draw(pngle_t *pngle, uint32_t x0, uint32_t y0, uint32_t x_n, uint32_t y_n, uint8_t rgba[4]) {
	if (! data) {
	    w = pngle_get_width(pngle);
	    h = pngle_get_height(pngle);

	    data = (uint8_t *) fatal_malloc(w * h * 4);
	}

	for (uint32_t y = y0; y < y0 + y_n; y++) {
	    for (uint32_t x = x0; x < x0 + x_n; x++) {
		uint8_t *store = &data[(x + y * w) * 4];
		store[0] = rgba[0];
		store[1] = rgba[1];
		store[2] = rgba[2];
		store[3] = rgba[3];
	    }
	}
    }

private:
    uint8_t *data = NULL;
};

class ImagePNGOnDraw : public OnDraw {
public:
    ImagePNGOnDraw(ImagePNG *image_png) : image_png(image_png) {}
    void on_draw(pngle_t *pngle, uint32_t x0, uint32_t y0, uint32_t x_n, uint32_t y_n, uint8_t rgba[4]) override {
	image_png->on_draw(pngle, x0, y0, x_n, y_n, rgba);
    }

private:
    ImagePNG *image_png;
};

static void static_on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4]) {
    OnDraw *drawer = (OnDraw *) pngle_get_user_data(pngle);
    drawer->on_draw(pngle, x, y, w, h, rgba);
}

#define BLOCK_SIZE 4096

Image *image_png_load(const char *fname) {
    file_t *f;

    if ((f = media_file_open_read(fname)) == NULL) {
	perror(fname);
	return NULL;
    }

    ImagePNG *image = new ImagePNG();
    ImagePNGOnDraw *on_draw = new ImagePNGOnDraw(image);

    pngle_t *pngle = pngle_new();
    pngle_set_draw_callback(pngle, static_on_draw);
    pngle_set_user_data(pngle, on_draw);

    uint8_t *block = (uint8_t *) fatal_malloc(BLOCK_SIZE);
    int n_block = 0 ;
    int n_read;

    while ((n_read = file_read(f, &block[n_block], BLOCK_SIZE - n_block)) > 0) {
	n_block += n_read;
	int n_consumed = pngle_feed(pngle, block, n_block);
	if (n_consumed < 0) {
	    fprintf(stderr, "%s: error %s\n", fname, pngle_error(pngle));
	    delete image;
	    delete on_draw;
	    return NULL;
	} else if (n_consumed == 0) {
	    break;
	} else {
	    if (n_block < n_consumed) memmove(block, &block[n_consumed], n_block - n_consumed);
	    n_block -= n_consumed;
	}
    }

    fatal_free(block);
    file_close(f);
    pngle_destroy(pngle);

    delete on_draw;

    return image;
}
