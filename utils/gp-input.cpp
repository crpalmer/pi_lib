#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "gp-input.h"
#include "pi.h"
#include "util.h"

static char buf[10*1024];
#define MAX_INPUTS 1000
static GPInput *inputs[MAX_INPUTS];
int n_inputs;


#ifdef PI_PICO

#include <pico/bootrom.h>

char *readline(char *buf, size_t n)
{
    pico_readline_echo(buf, n, true);
    printf("\n");
    return buf;
}

#else

#include <ctype.h>

char *readline(char *buf, size_t n)
{
    if (feof(stdin)) return NULL;
    if (fgets(buf, n, stdin) == NULL) return NULL;
    int i;
    for (i = strlen(buf); i > 0 && isspace(buf[i-1]); i--) {}
    buf[i] = '\0';
    return buf;
}

#endif

class Notifier : public GPInputNotifier {
public:
    Notifier(int id, unsigned gpio) {
	this->id = id;
	this->gpio = gpio;
    }
	
    virtual void on_change(bool is_rising, bool is_falling) {
	printf("%u [%d]: %d%s%s\n", gpio, id, inputs[id]->get(), is_rising ? " rising" : "", is_falling ? " falling" : "");
    }

private:
    int id;
    unsigned gpio;
};

int
main()
{
    pi_init();
    pi_gpio_init();

    while (readline(buf, sizeof(buf)) != NULL) {
	unsigned gpio;

	if (sscanf(buf, "%u", &gpio) == 1) {
	    inputs[n_inputs] = new GPInput(gpio);
	    inputs[n_inputs]->set_pullup_up();
	    if (inputs[n_inputs]->set_notifier(new Notifier(n_inputs, gpio)) < 0) {
	       perror("enable_irq");
	    } else {
	       n_inputs++;
	    }
#ifdef PI_PICO
	} else if (strcmp(buf, "bootsel") == 0) {
            printf("Rebooting into bootloader mode...\n");
            reset_usb_boot(1<<PICO_DEFAULT_LED_PIN, 0);
#endif
	} else if (buf[0] == '?') {
	    printf("gpio# - enable a watcher\n");
#ifdef PI_PICO
	    printf("bootsel\n");
#endif
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
    }
}