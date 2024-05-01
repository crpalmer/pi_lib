#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "i2c.h"
#include "mcp23017.h"
#include "pi.h"

static char buf[100*1024];

static MCP23017 *mcp;
static Input *inputs[2][8];
static Output *outputs[2][8];

static Input *ensure_input(unsigned bank, unsigned pin)
{
    assert(outputs[bank][pin] == NULL);
    if (inputs[bank][pin] == NULL) inputs[bank][pin] = mcp->get_input(bank, pin);
    return inputs[bank][pin];
}

static Output *ensure_output(unsigned bank, unsigned pin)
{
    assert(inputs[bank][pin] == NULL);
    if (outputs[bank][pin] == NULL) outputs[bank][pin] = mcp->get_output(bank, pin);
    return outputs[bank][pin];
}

int
main(int argc, char **argv)
{
    pi_init();

    i2c_init_bus(0);
    i2c_config_gpios(2, 3);

    pi_gpio_init();

#ifdef PLATFORM_pico
    if (0) {
#else
    if (argc > 1) {
#endif
	unsigned addr = atoi(argv[1]);
	fprintf(stderr, "using addr: 0x%02x\n", addr);
	mcp = new MCP23017(addr);
    } else {
	mcp = new MCP23017();
    }

    while (pi_readline(buf, sizeof(buf)) != NULL) {
	unsigned bank, pin;

	if (sscanf(&buf[1], "%d %d", &bank, &pin) == 2) {
	    switch (buf[0]) {
	    case '+':
		ensure_input(bank, pin)->set_pullup_up();
		break;
	    case '=':
		ensure_input(bank, pin)->clear_pullup();
		break;
	    case 'g': 
		printf("%d\n", ensure_input(bank, pin)->get());
		break;
	    case 'G':  {
		Input *input = ensure_input(bank, pin);
		unsigned cur = input->get();
		while (cur == input->get()) {}
		printf("%d\n", !cur);
		break;
	    }
	    case '1':
	    case '0':
		ensure_output(bank, pin)->set(buf[0] - '0');
		break;
	    }
	} else if (buf[0] == '?') {
	    printf("g bank pin : get value\nG bank pin : wait for value to change\n[1|0] bank pin : set value\n[+|=] bank pin : set/clear pullup\n");
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
    }
}
