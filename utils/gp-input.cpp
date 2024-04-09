#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "gp-input.h"
#include "pi.h"
#include "util.h"

static char buf[10*1024];

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

class MyGPInput : public GPInput {
public:
    MyGPInput(int id, unsigned gpio) : GPInput(gpio) {
	this->id = id;
	this->gpio = gpio;
	set_pullup_up();
    }
	
    virtual void on_change(bool is_rising, bool is_falling) {
	printf("%u [%d]: %d%s%s\n", gpio, id, get(), is_rising ? " rising" : "", is_falling ? " falling" : "");
    }

private:
    int id;
    unsigned gpio;
};

#if 0

static void test(unsigned gpio, void *arg_unused)
{
    printf("%d triggered: %d\n", gpio, pi_gpio_get(gpio));
    ms_sleep(10);
}

#endif

#define MAX_INPUTS 1000
static GPInput *inputs[MAX_INPUTS];
int n_inputs;

int
main()
{
    pi_init();
    pi_gpio_init();

#if 0
    if (pi_gpio_set_pullup(4, PI_PUD_UP) < 0) perror("pi_gpio_set_pullup");
    if (pi_gpio_set_irq_handler(4, test, NULL) < 0) perror("pi_gpio_set_irq_handler");
#endif

    while (readline(buf, sizeof(buf)) != NULL) {
	unsigned gpio;

	if (sscanf(buf, "%u", &gpio) == 1) {
	    inputs[n_inputs] = new MyGPInput(n_inputs, gpio);
	    if (inputs[n_inputs]->enable_irq() < 0) {
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
