#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mcp23017.h"

#define TRACE_WRITES		0

class MCP23017_input : public input_t {
public:
    MCP23017_input(MCP23017 *parent, unsigned bank, unsigned pin) : input_t() {
	this->parent = parent;
	this->bank   = bank;
	this->pin    = pin;
    }

    unsigned get_fast() override { return parent->get(bank, pin); }
    void set_pullup_up() override { parent->set_pullup(bank, pin, 1); }
    void clear_pullup() override { parent->set_pullup(bank, pin, 0); }
    void set_pullup_down() override {
#ifdef PI_PICO
	fprintf(stderr, "ERROR: MCP23017 cannot set a pulldown resistor\n");
#else
	throw "MCP23017 cannot set a pulldown resistor";
#endif
    }

private:
    MCP23017 *parent;
    unsigned bank;
    unsigned pin;
};

class MCP23017_output : public output_t {
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
    bus = i2cOpen(1, address, 0);
    if (bus < 0) {
#ifdef PI_PICO
	fprintf(stderr, "ERROR: failed to open i2c device\n");
#else
	throw "failed to open i2c device";
#endif
    }
    this->dir[0] = this->dir[1] = 0;
    this->pullup[0] = this->pullup[1] = 0;
    this->out[0] = this->out[1] = 0;

    write_out(0, true);
    write_out(1, true);
    write_dir(0, true);
    write_dir(1, true);
    write_pullup(0);
    write_pullup(1);
}

MCP23017::~MCP23017()
{
    i2cClose(bus);
}

unsigned MCP23017::get(unsigned bank, unsigned pin)
{
    assert_bank_pin(bank, pin);
    assert_input(bank, pin);
    return (i2cReadByteData(bus, in_val_addr[bank]) & pin_value(pin)) != 0;
}

void MCP23017::set(unsigned bank, unsigned pin, bool value)
{
    assert_bank_pin(bank, pin);
    assert_output(bank, pin);
    set_bit(out[bank], pin, value);
    write_out(bank);
}

void MCP23017::i2c_write(unsigned addr, unsigned value, bool validate)
{
#if TRACE_WRITES
    fprintf(stderr, "write 0x%02x = 0x%02x\n", addr, value);
#endif
    i2cWriteByteData(bus, addr, value);
    if (validate) {
	unsigned new_value = i2cReadByteData(bus, addr);
	if (value != new_value) {
	    fprintf(stderr, "I was supposed to write: 0x%02x but value is 0x%02x\n", value, new_value);
	    exit(1);
	}
    }
}

void MCP23017::write_dir(unsigned bank, bool validate)
{
    assert_bank(bank);
    i2c_write(dir_addr[bank], dir[bank], validate);
}

void MCP23017::write_pullup(unsigned bank, bool validate)
{
    assert_bank(bank);
    i2c_write(pullup_addr[bank], pullup[bank], validate);
}

void MCP23017::write_out(unsigned bank, bool validate)
{
    assert_bank(bank);
    i2c_write(out_val_addr[bank], out[bank], validate);
}

input_t *MCP23017::get_input(unsigned bank, unsigned pin)
{
    assert_bank_pin(bank, pin);
    set_bit(dir[bank], pin, 1);
    write_dir(bank);
    return new MCP23017_input(this, bank, pin);
}

output_t *MCP23017::get_output(unsigned bank, unsigned pin)
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
