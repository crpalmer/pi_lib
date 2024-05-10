#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pi.h"
#include "gp-input.h"
#include "i2c.h"
#include "mcp23017.h"

#define TRACE_WRITES		0

class MCPNotifier : public InputNotifier {
public:
    MCPNotifier(int gpio) {
	this->gpio = new GPInput(gpio);
	this->gpio->set_notifier(this);
    }

    virtual ~MCPNotifier() {
	delete gpio;
    }

    void on_change() {
	for (MCPInput *input : inputs) input->on_change();
    }

    void add(MCPInput *input) {
	inputs.remove(input);
	inputs.push_back(input);
    }

    void remove(MCPInput *input) {
	inputs.remove(input);
    }

private:
    Input *gpio;
    std::list<MCPInput *> inputs;
};

MCP23017::MCP23017(unsigned address, int int_A_gpio, int int_B_gpio) {
    i2c = i2c_open(1, address);
    if (i2c < 0) {
	fprintf(stderr, "failed to open i2c device\n");
    }
    dir[0] = dir[1] = 0;
    pullup[0] = pullup[1] = 0;
    out[0] = out[1] = 0;
    inten[0] = inten[1] = 0;

    notifier[0] = int_A_gpio >= 0 ? new MCPNotifier(int_A_gpio) : NULL;
    notifier[1] = int_B_gpio >= 0 ? new MCPNotifier(int_B_gpio) : NULL;

    write_out(0);
    write_out(1);
    write_dir(0);
    write_dir(1);
    write_pullup(0);
    write_pullup(1);
    write_inten(0);
    write_inten(1);
    i2c_write_byte(i2c, intcon_addr[0], 0);	// Make it all interrupt on change
    i2c_write_byte(i2c, intcon_addr[1], 0);
}

MCP23017::~MCP23017()
{
    i2c_close(i2c);
    if (notifier[0]) delete notifier[0];
    if (notifier[1]) delete notifier[1];
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

void MCP23017::write_inten(unsigned bank)
{
    assert_bank(bank);
    i2c_write_byte(i2c, inten_addr[bank], inten[bank]);
}

MCPInput *MCP23017::get_input(unsigned bank, unsigned pin)
{
    assert_bank_pin(bank, pin);
    set_bit(dir[bank], pin, 1);
    write_dir(bank);
    return new MCPInput(this, bank, pin);
}

MCPOutput *MCP23017::get_output(unsigned bank, unsigned pin)
{
    assert_bank_pin(bank, pin);
    set_bit(dir[bank], pin, 0);
    write_dir(bank);
    return new MCPOutput(this, bank, pin);
}

void MCP23017::set_pullup(unsigned bank, unsigned pin, unsigned up)
{
    assert_bank_pin(bank, pin);
    assert_input(bank, pin);
    set_bit(pullup[bank], pin, up);
    write_pullup(bank);
}

bool MCP23017::add_notifier(MCPInput *input, unsigned bus, unsigned pin) {
    if (bus > 1 || pin > 7) return false;
    if (! notifier[bus]) return false;
    notifier[bus]->add(input);
    inten[bus] |= (1 << pin);
    write_inten(bus);
    return true;
}

void MCP23017::remove_notifier(MCPInput *input, unsigned bus, unsigned pin) {
    if (bus > 1 || pin > 7) return;
    inten[bus] &= ~(1 << pin);
    write_inten(bus);
    notifier[bus]->remove(input);
}
