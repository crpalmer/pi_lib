#include <stdio.h>
#include <stdlib.h>
#include "i2c.h"

#include "ssd1306.h"

#define EXTERNAL_VCC 0
#define WIDTH 128
#define HEIGHT 64

#define COMMAND 0x00
#define DATA 0x40

// address modes

#define HORIZONTAL_ADDRESSING_MODE 0x00
#define VERTICAL_ADDRESSING_MODE 0x01
#define PAGE_ADDRESSING_MODE 0x02

// commands

#define SET_COLUMN_START_LOW_MASK 0x00
#define SET_COLUMN_START_HIGH_MASK 0x10
#define SET_MEMORY_ADDRESSING_MODE 0x20
#define SET_COLUMN_ADDRESS 0x21
#define SET_PAGE_ADDRESS 0x22
#define SET_DISPLAY_START_LINE_MASK 0x40
#define SET_CONTRAST 0x81
#define ENABLE_CHARGE_PUMP_REGULATOR 0x8D
#define SET_SEGMENT_REMAP_0 0xA0
#define SET_SEGMENT_REMAP_127 0xA1
#define SET_ENTIRE_DISPLAY_ON_RESUME 0xA4
#define SET_ENTIRE_DISPLAY_ON_FORCE 0xA5
#define SET_NORMAL_DISPLAY 0xA6
#define SET_INVERSE_DISPLAY 0xA7
#define SET_MUX_RATIO 0xA8
#define SET_DISPLAY_OFF 0xAE
#define SET_DISPLAY_ON 0xAF
#define SET_PAGE_START_ADDRESS_MASK 0xB0
#define SET_COM_OUTPUT_SCAN_DIRECTION_NORMAL 0xC0
#define SET_COM_OUTPUT_SCAN_DIRECTION_REMAP 0xC8
#define SET_DISPLAY_OFFSET 0xD3
#define SET_OSC_FREQUENCY 0xD5
#define SET_PRECHARGE_PERIOD 0xD9
#define SET_COM_PINS_HARDWARE_CONFIGURATION 0xDA
#define SET_VCOMH_DESELECT_LEVEL 0xDB

#define N_PAGES 8
#define PAGE_ROWS 8
#define PAGE_COLS WIDTH
#define BYTES_PER_PAGE (PAGE_ROWS * PAGE_COLS / 8)

/* ---------------------------------------------------------------------
 *
 * Page
 *
 * One page of display memory.
 */

class Page {
public:
    Page() {
	raw = (uint8_t *) fatal_malloc(BYTES_PER_PAGE);
	memset(raw, 0, BYTES_PER_PAGE);
    }

    ~Page() {
	fatal_free(raw);
    }

    RGB32 get_pixel(int x, int y) {
	uint8_t *p = &raw[byte_of(x, y)];
	return (*p & (1 << bit_of(x, y))) != 0;
    }

    void set_pixel(int x, int y, int value) {
	uint8_t *p = &raw[byte_of(x, y)];
	int bit_value = 1<<bit_of(x, y);

	if (value) *p |= bit_value;
	else *p &= ~bit_value;

	dirty = true;
    }

    uint8_t *get_raw()
    {
	dirty = false;
        return raw;
    }

    bool is_dirty() { return dirty; }

private:
    int byte_of(int x, int y) { return x; }
    int bit_of(int x, int y) { return y; }

    uint8_t *raw;
    bool dirty = true;
};

/* ---------------------------------------------------------------------
 *
 * Canvas
 *
 */

class SSD1306_Canvas : public Canvas {
public:
    SSD1306_Canvas() {
	w = 128;
	h = 64;
	bpp = 0;
	for (int page = 0; page < N_PAGES; page++) pages[page] = new Page();
    }

    ~SSD1306_Canvas() {
	for (int page = 0; page < N_PAGES; page++) {
	    delete pages[page];
	}
    }

    RGB32 get_pixel(int x, int y) {
	Page *page = get_page(y);
	return page->get_pixel(x, row_of_page(y));
    }

    void set_pixel(int x, int y, Byte r, Byte g, Byte b) {
	Page *page = get_page(y);
	page->set_pixel(x, row_of_page(y), r + g + b > 0);
    }

    uint8_t *get_raw_if_dirty(int page)
    {
	if (pages[page]->is_dirty()) return pages[page]->get_raw();
	else return NULL;
    }

private:
    Page *pages[N_PAGES];

    Page *get_page(int row) {
	int page = row / PAGE_ROWS;
	return pages[page];
    }

    int row_of_page(int row) {
	 return row % PAGE_ROWS;
    }
};

/* ---------------------------------------------------------------------
 *
 * SSD1306
 *
 */

void SSD1306::write_cmd(uint8_t cmd)
{
    i2c_write_byte(fd, COMMAND, cmd);
}

void SSD1306::write_cmd(uint8_t cmd, uint8_t v1)
{
    unsigned char data[2] = { cmd, v1 };
    i2c_write(fd, COMMAND, data, 2);
}

void SSD1306::write_cmd(uint8_t cmd, uint8_t v1, uint8_t v2)
{
    unsigned char data[3] = { cmd, v1, v2 };
    i2c_write(fd, COMMAND, data, 3);
}

void SSD1306::write_data(uint8_t *data, int n_data)
{
    i2c_write(fd, DATA, data, n_data);
}

SSD1306::SSD1306(int bus, int addr)
{
    fd = i2c_open(bus, addr);

    write_cmd(ENABLE_CHARGE_PUMP_REGULATOR, 0x14);
    write_cmd(SET_MEMORY_ADDRESSING_MODE, PAGE_ADDRESSING_MODE);
    write_cmd(SET_OSC_FREQUENCY, 0x80);
    write_cmd(SET_DISPLAY_OFFSET, 0x00);
    write_cmd(SET_DISPLAY_START_LINE_MASK | 0x00);
    write_cmd(SET_SEGMENT_REMAP_127);
    write_cmd(SET_COM_OUTPUT_SCAN_DIRECTION_REMAP);
    write_cmd(SET_COM_PINS_HARDWARE_CONFIGURATION, 0x12);
    write_cmd(SET_PRECHARGE_PERIOD, 0xF1);
    write_cmd(SET_VCOMH_DESELECT_LEVEL, 0x40);
    write_cmd(SET_ENTIRE_DISPLAY_ON_RESUME);
    write_cmd(SET_NORMAL_DISPLAY);
    write_cmd(SET_COLUMN_ADDRESS, 0x00, 0x7F);
    write_cmd(SET_PAGE_ADDRESS, 0x00, 0x07);
    write_cmd(SET_CONTRAST, 0x7F);
    write_cmd(SET_DISPLAY_ON);
}

Canvas *SSD1306::create_canvas()
{
    return new SSD1306_Canvas();
}

void SSD1306::set_brightness(double pct)
{
}

void SSD1306::paint(Canvas *canvas)
{
    SSD1306_Canvas *c = (SSD1306_Canvas *) canvas;

    for (int page = 0; page < N_PAGES; page++) {
	uint8_t *raw = c->get_raw_if_dirty(page);
	if (raw) {
	    write_cmd(SET_PAGE_START_ADDRESS_MASK | page);
	    write_cmd(SET_COLUMN_START_LOW_MASK | 0);
	    write_cmd(SET_COLUMN_START_HIGH_MASK | 0);
	    write_data(raw, BYTES_PER_PAGE);
	}
    }
}
