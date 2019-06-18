#include <stdio.h>
#include <assert.h>
#include "pigpio.h"
#include "mcp23017.h"

static char buf[100*1024];

static MCP23017 *mcp;
static input_t *inputs[2][8];
static output_t *outputs[2][8];

static input_t *ensure_input(unsigned bank, unsigned pin)
{
    assert(outputs[bank][pin] == NULL);
    if (inputs[bank][pin] == NULL) inputs[bank][pin] = mcp->get_input(bank, pin);
    return inputs[bank][pin];
}

static output_t *ensure_output(unsigned bank, unsigned pin)
{
    assert(inputs[bank][pin] == NULL);
    if (outputs[bank][pin] == NULL) outputs[bank][pin] = mcp->get_output(bank, pin);
    return outputs[bank][pin];
}

int
main(int argc, char **argv)
{
    gpioInitialise();
    mcp = new MCP23017();

    while (fgets(buf, sizeof(buf), stdin) != NULL && ! feof(stdin)) {
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
		input_t *input = ensure_input(bank, pin);
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
