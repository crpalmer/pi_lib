#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mem.h"
#include "pi.h"

#include "image-png.h"

static void static_on_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4]) {
    ImagePNG *image_png = (ImagePNG *) pngle_get_user_data(pngle);
    image_png->on_draw(x, y, w, h, rgba);
}

void ImagePNG::on_draw(uint32_t x0, uint32_t y0, uint32_t x_n, uint32_t y_n, uint8_t rgba[4]) {
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

#define BLOCK_SIZE 4096

ImagePNG::ImagePNG(const char *fname)
{
    file_t *f;

    if ((f = media_file_open_read(fname)) == NULL) {
	perror(fname);
	w = h = 0;
	return;
    }

    data = NULL;

    pngle = pngle_new();
    pngle_set_draw_callback(pngle, static_on_draw);
    pngle_set_user_data(pngle, this);

    uint8_t *block = (uint8_t *) fatal_malloc(BLOCK_SIZE);
    int n_block = 0 ;
    int n_read;

    while ((n_read = file_read(f, &block[n_block], BLOCK_SIZE - n_block)) > 0) {
	n_block += n_read;
	int n_consumed = pngle_feed(pngle, block, n_block);
	if (n_consumed < 0) {
	    // TODO handle the error
	    fprintf(stderr, "%s: error %s\n", fname, pngle_error(pngle));
	    break;
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
}

ImagePNG::~ImagePNG() {
    fatal_free(data);
}

bool ImagePNG::get_pixel(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b)
{
    uint8_t *rgb = &data[(x + y*w) * 4];

if (x == 0 && y == 60) printf("data get %p\n", data);
    if (rgb[3] == 0) return false;

if (x == 0 && y == 60)
printf("rgb %p\n", rgb);
    *r = rgb[0];
    *g = rgb[1];
    *b = rgb[2];

    return true;
}
