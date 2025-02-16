#include <stdio.h>
#include <string.h>
#include "pi.h"
#include "mem.h"
#include "spi.h"
#include "time-utils.h"
#include "il9341.h"

const uint8_t RDDMADCTL = 0x0b;
const uint8_t RDDMADCTL_BOTTOM_TO_TOP = 0x80;
const uint8_t RDDMADCTL_RIGHT_TO_LEFT = 0x40;
const uint8_t RDDMADCTL_REVERSE = 0x20;
const uint8_t RDDMADCTL_LCD_BOTTOM_TO_TOP = 0x10;
const uint8_t RDDMADCTL_BGR = 0x08;
const uint8_t RDDMADCTL_LCD_RIGHT_TO_LEFT = 0x04;

const uint8_t SLEEP_OUT = 0x11;
const uint8_t DISPLAY_INVERSION_ON = 0x21;
const uint8_t GAMMA_SET = 0x26;
const uint8_t DISPLAY_ON = 0x29;
const uint8_t COLUMN_ADDRESS_SET = 0x2a;
const uint8_t PAGE_ADDRESS_SET = 0x2b;
const uint8_t MEMORY_WRITE = 0x2c;

const uint8_t MEMORY_ACCESS_CONTROL = 0x36;
const uint8_t MEMORY_ACCESS_CONTROL_MY = 0x80;
const uint8_t MEMORY_ACCESS_CONTROL_MX = 0x40;
const uint8_t MEMORY_ACCESS_CONTROL_MV = 0x20;
const uint8_t MEMORY_ACCESS_CONTROL_ML = 0x10;
const uint8_t MEMORY_ACCESS_CONTROL_BGR = 0x08;
const uint8_t MEMORY_ACESS_CONTROL_MH = 0x04;

const uint8_t VERTICAL_SCROLLING_START = 0x37;
const uint8_t IDLE_OFF = 0x38;
const uint8_t COLMOD = 0x3a;
const uint8_t FRAME_RATE_CONTROL_NORMAL = 0xb1;
const uint8_t FRAME_RATE_CONTROL_IDLE = 0xb2;
const uint8_t DISPLAY_CONTROL_FUNCTION = 0xb6;
const uint8_t ENTRY_MODE_SET = 0xb7;
const uint8_t BACKLIGHT_CONTROL_4 = 0xbb;
const uint8_t POWER_CONTROL_1 = 0xc0;
const uint8_t POWER_CONTROL_2 = 0xc1;
const uint8_t VCOM_CONTROL_1 = 0xc5;
const uint8_t VCOM_CONTROL_2 = 0xc6;
const uint8_t POWER_CONTROL_A = 0xcb;
const uint8_t POWER_CONTROL_B = 0xcf;
const uint8_t NV_MEMORY_WRITE = 0xd0;
const uint8_t POSITIVE_GAMMA_CORRECTION = 0xe0;
const uint8_t NEGATIVE_GAMMA_CORRECTION = 0xe1;
const uint8_t DRIVER_TIMING_CONTROL_A = 0xe8;
const uint8_t DRIVER_TIMING_CONTROL_B = 0xea;
const uint8_t POWER_ON_SEQUENCE = 0xed;
const uint8_t ENABLE_3G = 0xf2;
const uint8_t PUMP_RATIO_CONTROL = 0xf7;

const int MAX_DATA = 16;
const struct {
    uint8_t cmd;
    uint8_t n_data;
    uint8_t data[MAX_DATA];
} init_cmds[] = {
    0xEF, 3, { 0x03, 0x80, 0x02 },
    POWER_CONTROL_B, 3, { 0x00, 0xC1, 0x30 },
    POWER_ON_SEQUENCE, 4, { 0x64, 0x03, 0x12, 0x81 },
    DRIVER_TIMING_CONTROL_A, 3, { 0x85, 0x00, 0x78 },
    POWER_CONTROL_A, 5, { 0x39, 0x2C, 0x00, 0x34, 0x02 },
    PUMP_RATIO_CONTROL, 1, { 0x20 },
    DRIVER_TIMING_CONTROL_B, 2, { 0x00, 0x00 },
    POWER_CONTROL_1, 1, { 0x23},	   	// Power control VRH[5:0]
    POWER_CONTROL_2, 1, { 0x10},	   	// Power control SAP[2:0];BT[3:0]
    VCOM_CONTROL_1, 2, { 0x3e, 0x28},
    VCOM_CONTROL_2, 1, { 0x86},
    RDDMADCTL, 2, { RDDMADCTL_RIGHT_TO_LEFT | RDDMADCTL_BGR, RDDMADCTL_RIGHT_TO_LEFT | RDDMADCTL_BGR },		   	// Memory Access Control
    MEMORY_ACCESS_CONTROL, 1, { MEMORY_ACCESS_CONTROL_MV | MEMORY_ACCESS_CONTROL_BGR },
    VERTICAL_SCROLLING_START, 0, { 0x00 },	// Vertical scroll zero
    COLMOD, 1, { 0x55 },			// 16bpp
    FRAME_RATE_CONTROL_NORMAL, 2, { 0x00, 0x1b },
    DISPLAY_CONTROL_FUNCTION, 3, { 0x08, 0x82, 0x27 },					 // Display Function Control
    ENABLE_3G, 1, { 0x00 },								 // 3Gamma Function Disable
    GAMMA_SET, 1, { 0x01 },								 // Gamma curve selected
    POSITIVE_GAMMA_CORRECTION, 15, { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 },
    NEGATIVE_GAMMA_CORRECTION, 15, { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F },
};
const int n_init_cmds = sizeof(init_cmds) / sizeof(init_cmds[0]);

void IL9341::reset() {
    reset_pin->set(0);
    ms_sleep(5);
    reset_pin->set(1);
    ms_sleep(150);
}

class IL9341_Canvas : public Canvas {
public:
    IL9341_Canvas(IL9341 *display, int w, int h) : Canvas(w, h), display(display) {
	data = (uint16_t *) fatal_malloc(w * sizeof(*data));
	dirty_x = -1;
    }

    ~IL9341_Canvas() {
	fatal_free(data);
    }

    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) override {
	if (dirty_x >= 0 && (dirty_y != y || x < dirty_x || x > dirty_x_max+1)) {
	    flush();
	}

	if (dirty_x < 0) {
	    dirty_x = dirty_x_max = x;
	    dirty_y = y;
	}

	if (x > dirty_x_max) dirty_x_max = x;

	data[x] = RGB16_of(r, g, b);
    }

    void flush() override {
	if (dirty_x >= 0) {
	    display->draw(dirty_x, dirty_y, dirty_x_max, dirty_y, &data[dirty_x]);
	    dirty_x = -1;
	}
    }

private:
    int dirty_x, dirty_y, dirty_x_max;
    IL9341 *display;
    uint16_t *data;
};

class IL9341_BufferedCanvas : public Canvas {
public:
    IL9341_BufferedCanvas(IL9341 *display, int w, int h) : Canvas(w, h), display(display) {
	data = (uint16_t *) fatal_malloc(w * h * sizeof(*data));
	memset(data, 0, w * h * sizeof(*data));
    }

    ~IL9341_BufferedCanvas() {
	fatal_free(data);
    }

    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) override {
	data[x + y*w] = RGB16_of(r, g, b);
    }

    void flush() override {
	display->draw(0, 0, w-1, h-1, data);
    }

private:
    IL9341 *display;
    uint16_t *data;
};

Canvas *IL9341::create_canvas(bool prefer_unbuffered) {
    if (prefer_unbuffered) {
	return new IL9341_Canvas(this, width, height);
    } else {
	return new IL9341_BufferedCanvas(this, width, height);
    }
}

void IL9341::set_brightness(double pct) {
    if (pct < 0.5) backlight->off();
    else backlight->on();
}

void IL9341::draw(int x0, int y0, int x_max, int y_max, uint16_t *raw) {
    uint8_t column_address[4] = { (uint8_t) (x0 >> 8), (uint8_t) (x0 & 0xff), (uint8_t) (x_max >> 8), (uint8_t) (x_max & 0xff) };
    uint8_t row_address[4] = { (uint8_t) (y0 >> 8), (uint8_t) (y0 & 0xff), (uint8_t) (y_max >> 8), (uint8_t) (y_max & 0xff) };

    spi->write_cmd(COLUMN_ADDRESS_SET);
    spi->write_data(column_address, 4);

    spi->write_cmd(PAGE_ADDRESS_SET);
    spi->write_data(row_address, 4);

    spi->write_cmd(MEMORY_WRITE);
    spi->write_data16(raw, (x_max - x0 + 1)*(y_max - y0 + 1));
}

IL9341::IL9341(SPI *spi, Output *reset_pin, Output *backlight, int width, int height) : spi(spi), reset_pin(reset_pin), backlight(backlight), width(width), height(height) {
    backlight->off();

    reset();

    for (int i = 0; i < n_init_cmds; i++) {
	spi->write_cmd(init_cmds[i].cmd);
	if (init_cmds[i].n_data) spi->write_data(init_cmds[i].data, init_cmds[i].n_data);
    }

    spi->write_cmd(SLEEP_OUT);
    ms_sleep(150);
    spi->write_cmd(DISPLAY_ON);
    ms_sleep(150);

    backlight->on();
}
