#include <stdio.h>
#include "pi.h"
#include "mem.h"
#include "spi.h"
#include "time-utils.h"
#include "il9341.h"

const uint8_t RDDMADCTL = 0x0b;
const uint8_t SLEEP_OUT = 0x11;
const uint8_t DISPLAY_INVERSION_ON = 0x21;
const uint8_t GAMMA_SET = 0x26;
const uint8_t DISPLAY_ON = 0x29;
const uint8_t COLUMN_ADDRESS_SET = 0x2a;
const uint8_t PAGE_ADDRESS_SET = 0x2b;
const uint8_t MEMORY_WRITE = 0x2c;
const uint8_t MEMORY_ACCESS_CONTROL = 0x36;
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
    RDDMADCTL, 1, { 0x48 },		   	// Memory Access Control
    MEMORY_ACCESS_CONTROL, 1, { 8 },
    VERTICAL_SCROLLING_START, 0, { 0x00 },	// Vertical scroll zero
    COLMOD, 1, { 0x55 },			// 16bpp
    FRAME_RATE_CONTROL_NORMAL, 2, { 0x00, 0x18 },
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
    IL9341_Canvas(int w, int h) {
	this->w = w;
	this->h = h;
	this->bpp = 16;

	data = (uint16_t *) fatal_malloc(w * h * sizeof(*data));
    }

    ~IL9341_Canvas() {
	fatal_free(data);
    }

    RGB16 get_pixel16(int x, int y) override {
	return byte_swap(data[y*w + x]);
    }

    void set_pixel(int x, int y, Byte r, Byte g, Byte b) override {
	data[y*w + x] = byte_swap(RGB16_of(r, g, b));
    }

    RGB16 *get_raw() { return data; }

private:
    RGB16 *data;

    RGB16 byte_swap(RGB16 rgb) {
	//return (rgb & 0xff) << 8 | (rgb >> 8);
	return rgb;
    }
};

Canvas *IL9341::create_canvas() {
    return new IL9341_Canvas(width, height);
}

void IL9341::set_brightness(double pct) {
    if (pct < 0.5) backlight->off();
    else backlight->on();
}

void IL9341::paint(Canvas *generic_canvas) {
    IL9341_Canvas *canvas = (IL9341_Canvas *) generic_canvas;

    int end_column = width-1;
    int end_row = height-1;
    uint8_t column_address[4] = { 0, 0, (uint8_t) (end_column >> 8), (uint8_t) (end_column & 0xff) };
    uint8_t row_address[4] = { 0, 0, (uint8_t) (end_row >> 8), (uint8_t) (end_row & 0xff) };

    spi->write_cmd(COLUMN_ADDRESS_SET);
    spi->write_data(column_address, 4);

    spi->write_cmd(PAGE_ADDRESS_SET);
    spi->write_data(row_address, 4);

RGB24 rgb = canvas->get_pixel(200, 211);
printf("%02x %02x %02x\n", RGB24_r(rgb), RGB24_g(rgb), RGB24_b(rgb));
    spi->write_cmd(MEMORY_WRITE);
    spi->write_data16(canvas->get_raw(), width * height);
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
