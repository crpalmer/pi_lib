#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "pi.h"
#include "i2c.h"
#include "gp-input.h"
#include "mcp23017.h"
#include "mem.h"

static char buf[10*1024];
static MCP23017 *mcp = NULL;

class Notifier : public InputNotifier {
public:
    Notifier(Input *input, const char *who) : input(input), who(fatal_strdup(who)) { }
    ~Notifier() { fatal_free(who); }
	
    void on_change(bool is_rising, bool is_falling) override {
	printf("%s: %d%s%s\n", who, input->get(), is_rising ? " rising" : "", is_falling ? " falling" : "");
    }

    void on_change() override {
	printf("%s: %d\n", who, input->get());
    }

private:
    Input *input;
    char *who;
};

static void ensure_mcp() {
    if (mcp) return;

    printf("Initializing mcp...\n");
    i2c_init_bus(1);
    i2c_config_gpios(2, 3);

    mcp = new MCP23017(MCP23017_DEFAULT_ADDRESS, 6, 7);
    printf("Initialized.\n");
}

int
main()
{
    pi_init();

    printf("Input notifier is ready, specify the inputs to monitor.\n");
    while (pi_readline(buf, sizeof(buf)) != NULL) {
	unsigned gpio, bus, pin;

	if (strncmp(buf, "gpio ", 5) == 0 && sscanf(&buf[5], "%u", &gpio) == 1) {
	    GPInput *input = new GPInput(gpio);
	    input->set_pullup_up();
	    
	    if (! input->set_notifier(new Notifier(input, buf))) {
	        perror("enable_irq");
		delete input;
	    }
	} else if (strncmp(buf, "mcp ", 4) == 0 && sscanf(&buf[4], "%u %u", &bus, &pin) == 2) {
	    ensure_mcp();
	    MCPInput *input = mcp->get_input(bus, pin);
	    input->set_pullup_up();

	    if (! input->set_notifier(new Notifier(input, buf))) {
		printf("Failed to set notifier.\n");
		delete input;
	    }
	} else if (strcmp(buf, "bootsel") == 0) {
            pi_reboot_bootloader();
	} else {
	    if (buf[0] != '?') printf("Invalid command.  ");
	    printf("Usage:\n   gpio <gpio#>\n   mcp <bus#> <pin#>\n   bootsel\n");
	}
    }
}
