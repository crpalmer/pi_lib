#ifndef __MCP23017_H__
#define __MCP23017_H__

#include <assert.h>

#include "io.h"
#include "pigpio.h"

class MCP23017_input;
class MCP23017_output;

class MCP23017 {
    friend MCP23017_input;
    friend MCP23017_output;

public:
    MCP23017(unsigned address = 0x27);
    ~MCP23017();

    input_t *get_input(unsigned bank, unsigned pin);
    output_t *get_output(unsigned bank, unsigned pin);

protected:
    unsigned get(unsigned bank, unsigned pin);
    void set_pullup(unsigned bank, unsigned pin, unsigned up);
    void set(unsigned bank, unsigned pin, unsigned value);

private:
    void write_dir(unsigned bank);
    void write_pullup(unsigned bank);
    void write_out(unsigned bank);

    int bus;
    unsigned dir[2];
    unsigned pullup[2];
    unsigned out[2];

    const unsigned dir_addr[2]     = {0x00, 0x01};
    const unsigned out_val_addr[2] = {0x14, 0x15};
    const unsigned in_val_addr[2]  = {0x12, 0x13};
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

#endif
