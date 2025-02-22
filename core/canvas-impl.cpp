#include "pi.h"
#include "canvas-impl.h"

void UnbufferedCanvas::set_pixel_raw(int x, int y, uint8_t *pixel) {
    if (needs_to_be_flushed(x, y)) flush();

    if (dirty_x < 0) {
	dirty_x = dirty_first_x_max = dirty_cur_x_max = x;
	dirty_y = dirty_y_max = y;
    }

    if (y == dirty_y_max+1) {
	dirty_y_max = y;
	dirty_cur_x_max = x;
    }

    if (x > dirty_cur_x_max) {
	if (dirty_y == dirty_y_max) dirty_first_x_max = x;
	dirty_cur_x_max = x;
    }

    unsigned char *store = dirty_at(x, y);
    for (int i = 0; i < bytes_per_pixel; i++) store[i] = pixel[i];
}

bool UnbufferedCanvas::needs_to_be_flushed(int x, int y) {
    if (dirty_x < 0) return false;			// First pixel, no flush

    if (x < dirty_x) return true;			// To the left of the dirty window
    if (x > dirty_cur_x_max + 1) return true;	// Skipping pixels on this line
    if (y > dirty_y_max && dirty_cur_x_max < dirty_first_x_max) return true;  // Didn't finish the pixels on this line
    if (dirty_y < dirty_y_max && x > dirty_first_x_max) return true;  // To the right of the dirty_window
    if (y < dirty_y) return true;			// Above the window
    if (y > dirty_y_max+1) return true;		// Below the dirty window

    // Check that we have space for the new line
    if (y > dirty_y_max && (dirty_first_x_max - dirty_x + 1) * (y - dirty_y + 1) > w*buffered_rows) return true;

    return false;
}

void UnbufferedCanvas::flush() {
    if (dirty_x >= 0) {
	if (dirty_cur_x_max == dirty_first_x_max) {
	    display->draw(dirty_x, dirty_y, dirty_first_x_max, dirty_y_max, dirty);
	} else {
	    display->draw(dirty_x, dirty_y, dirty_first_x_max, dirty_y_max-1, dirty);
	    display->draw(dirty_x, dirty_y_max, dirty_cur_x_max, dirty_y_max, dirty_at(dirty_x, dirty_y_max));
	}
	dirty_x = -1;
    }
}

uint8_t *UnbufferedCanvas::dirty_at(int x, int y) {
    x -= dirty_x;
    y -= dirty_y;
    int x_per_row = (dirty_first_x_max - dirty_x + 1);
    return &dirty[(x + y*x_per_row) * bytes_per_pixel];
}

