#ifndef __MCP23017_H__
#define __MCP23017_H__

#include <assert.h>

#include <list>
#include "io.h"
#include "pi-gpio.h"

#define VALIDATE_WRITES  false

class MCPInput;
class MCPNotifier;
class MCPOutput;

#define MCP23017_DEFAULT_ADDRESS 0x27

class MCP23017 {
    friend MCPInput;
    friend MCPOutput;

public:
    MCP23017(unsigned address = MCP23017_DEFAULT_ADDRESS, int int_A_gpio = -1, int int_B_gpio = -1);
    virtual ~MCP23017();

    MCPInput *get_input(unsigned bank, unsigned pin);
    MCPInput *get_input(unsigned pin) { return get_input(pin / 8, pin % 8); }
    MCPOutput *get_output(unsigned bank, unsigned pin);
    MCPOutput *get_output(unsigned pin) { return get_output(pin / 8, pin % 8); }

protected:
    unsigned get(unsigned bank, unsigned pin);
    void set_pullup(unsigned bank, unsigned pin, unsigned up);
    void set(unsigned bank, unsigned pin, bool value);

    bool add_notifier(MCPInput *input, unsigned bus, unsigned pin);
    void remove_notifier(MCPInput *input, unsigned bus, unsigned pin);

private:
    void write_dir(unsigned bank);
    void write_pullup(unsigned bank);
    void write_out(unsigned bank);
    void write_inten(unsigned bank);

    int i2c;
    unsigned dir[2];
    unsigned pullup[2];
    unsigned out[2];
    unsigned inten[2];

    MCPNotifier *notifier[2];

    const unsigned dir_addr[2]     = {0x00, 0x01};
    const unsigned inten_addr[2]   = {0x04, 0x05};
    const unsigned intcon_addr[2]  = {0x08, 0x09};
    const unsigned in_val_addr[2]  = {0x12, 0x13};
    const unsigned out_val_addr[2] = {0x14, 0x15};
    const unsigned pullup_addr[2]  = {0x0c, 0x0d};

    unsigned pin_value(unsigned pin) { return 1 << pin; }

    void set_bit(unsigned &bits, unsigned bit, unsigned value)
    {
        assert(value == 0 || value == 1);
        bits = (bits & ~pin_value(bit)) | (value << bit);
    }


    void assert_input(unsigned bank, unsigned pin) { assert((dir[bank] & pin_value(pin)) != 0); }
    void assert_output(unsigned bank, unsigned pin) { assert((dir[bank] & pin_value(pin)) == 0); }
    void assert_bank(unsigned bank) { assert(bank == 0 || bank == 1); }
    void assert_pin(unsigned pin)   { assert(pin < 8); }
    void assert_bank_pin(unsigned bank, unsigned pin)
    {
        assert_bank(bank);
        assert_pin(pin);
    }
};

class MCPInput : public Input, InputNotifier {
public:
    MCPInput(MCP23017 *parent, unsigned bank, unsigned pin) : Input() {
        this->parent = parent;
        this->bank   = bank;
        this->pin    = pin;
    }
    ~MCPInput() {
	parent->remove_notifier(this, bank, pin);
    }

    unsigned get_fast() override { return parent->get(bank, pin); }
    void set_pullup_up() override { parent->set_pullup(bank, pin, 1); }
    void clear_pullup() override { parent->set_pullup(bank, pin, 0); }
    void set_pullup_down() override {
        fprintf(stderr, "ERROR: MCP23017 cannot set a pulldown resistor\n");
    }

    bool set_notifier(InputNotifier *notifier) override {
	if (! parent->add_notifier(this, bank, pin)) return false;
	this->notifier = notifier;
	return true;
    }

    void on_change() override {
	if (notifier) notifier->on_change();
    }

private:
    MCP23017 *parent;
    unsigned bank;
    unsigned pin;
    InputNotifier *notifier = NULL;
};

class MCPOutput : public Output {
public:
    MCPOutput(MCP23017 *parent, unsigned bank, unsigned pin) {
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

#endif
