#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <png.h>
#include "file.h"
#include "mem.h"

#include "canvas_png.h"

CanvasPNG::CanvasPNG(const char *fname)
{
    FILE *f;
    unsigned char header[8];

    had_error = 0;

    if ((f = media_fopen_read(fname)) == NULL) {
fail:
	perror(fname);
	w = h = 0;
	had_error = 1;
	return;
    }

    if (fread(header, 1, 8, f) != 8) {
	fprintf(stderr, "%s: Failed to read header\n", fname);
	goto fail;
    }

    if (png_sig_cmp(header, 0, 8)) {
	fprintf(stderr, "%s: not a valid png file\n", fname);
	goto fail;
    }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "%s: Something went wrong processing the data\n", fname);
	goto fail;
    }

    png_init_io(png, f);
    png_set_sig_bytes(png, 8);

    png_read_info(png, info);

    w = png_get_image_width(png, info);
    h = png_get_image_height(png, info);
 
    png_set_interlace_handling(png);
    png_read_update_info(png, info);

    if (setjmp(png_jmpbuf(png))) {
        fprintf(stderr, "%s: Something went wrong processing the data.\n", fname); 
	goto fail;
    }

    data = (png_bytep *) fatal_malloc(sizeof(*data) * h);

    for (int y = 0; y < h; y++) {
	data[y] = (png_byte *) fatal_malloc(png_get_rowbytes(png, info));
    }

    png_read_image(png, data);

    switch(png_get_color_type(png, info)) {
    case PNG_COLOR_TYPE_RGB: bytes_per_pixel = 3; break;
    case PNG_COLOR_TYPE_RGBA: bytes_per_pixel = 4; break;
    default: assert(0);
    }

    fclose(f);
}

RGB32 CanvasPNG::get_pixel(int x, int y)
{
    png_byte *line = data[y];
    png_byte *rgb = &line[x*bytes_per_pixel];

    if (bytes_per_pixel >= 4 && rgb[3] == 0) return 0;
    else return rgb32(rgb[0], rgb[1], rgb[2]);
}

void CanvasPNG::set_pixel(int x, int y, Byte r, Byte g, Byte b)
{
    png_byte *line = data[y];
    png_byte *rgb = &line[x*bytes_per_pixel];
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

