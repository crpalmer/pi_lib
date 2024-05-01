#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "i2c.h"
#include "mcp23017.h"

#define TRACE_WRITES		0

class MCP23017_input : public Input {
public:
    MCP23017_input(MCP23017 *parent, unsigned bank, unsigned pin) : Input() {
	this->parent = parent;
	this->bank   = bank;
	this->pin    = pin;
    }

    unsigned get_fast() override { return parent->get(bank, pin); }
    void set_pullup_up() override { parent->set_pullup(bank, pin, 1); }
    void clear_pullup() override { parent->set_pullup(bank, pin, 0); }
    void set_pullup_down() override {
	fprintf(stderr, "ERROR: MCP23017 cannot set a pulldown resistor\n");
    }

private:
    MCP23017 *parent;
    unsigned bank;
    unsigned pin;
};

class MCP23017_output : public Output {
public:
    MCP23017_output(MCP23017 *parent, unsigned bank, unsigned pin) {
	this->parent = parent;
	this->bank = bank;
	this->pin = pin;
    }

    void set(bool value) override { parent->set(bank, pin, value); }

private:
    MCP23017 *parent;
    unsigned bank;
    unsigned pin;
};

MCP23017::MCP23017(unsigned address)
{
    i2c = i2c_open(1, address);
    if (i2c < 0) {
	fprintf(stderr, "failed to open i2c device\n");
    }
    this->dir[0] = this->dir[1] = 0;
    this->pullup[0] = this->pullup[1] = 0;
    this->out[0] = this->out[1] = 0;

    write_out(0);
    write_out(1);
    write_dir(0);
    write_dir(1);
    write_pullup(0);
    write_pullup(1);
}

MCP23017::~MCP23017()
{
    i2c_close(i2c);
}

unsigned MCP23017::get(unsigned bank, unsigned pin)
{
    assert_bank_pin(bank, pin);
    assert_input(bank, pin);
    unsigned char data;
    i2c_read_byte(i2c, in_val_addr[bank], &data);
    return (data & pin_value(pin)) != 0;
}

void MCP23017::set(unsigned bank, unsigned pin, bool value)
{
    assert_bank_pin(bank, pin);
    assert_output(bank, pin);
    set_bit(out[bank], pin, value);
    write_out(bank);
}

void MCP23017::write_dir(unsigned bank)
{
    assert_bank(bank);
    i2c_write_byte(i2c, dir_addr[bank], dir[bank]);
}

void MCP23017::write_pullup(unsigned bank)
{
    assert_bank(bank);
    i2c_write_byte(i2c, pullup_addr[bank], pullup[bank]);
}

void MCP23017::write_out(unsigned bank)
{
    assert_bank(bank);
    i2c_write_byte(i2c, out_val_addr[bank], out[bank]);
}

Input *MCP23017::get_input(unsigned bank, unsigned pin)
{
    assert_bank_pin(bank, pin);
    set_bit(dir[bank], pin, 1);
    write_dir(bank);
    return new MCP23017_input(this, bank, pin);
}

Output *MCP23017::get_output(unsigned bank, unsigned pin)
{
    assert_bank_pin(bank, pin);
    set_bit(dir[bank], pin, 0);
    write_dir(bank);
    return new MCP23017_output(this, bank, pin);
}

void MCP23017::set_pullup(unsigned bank, unsigned pin, unsigned up)
{
    assert_bank_pin(bank, pin);
    assert_input(bank, pin);
    set_bit(pullup[bank], pin, up);
    write_pullup(bank);
}
