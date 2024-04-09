#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "pi.h"
#include "rotary-encoder.h"
#include "util.h"

static char buf[10*1024];
static RotaryEncoder *re;
static int min_value = -INT_MAX;
static int max_value = +INT_MAX;


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

class Notifier : public RotaryEncoderNotifier {
public:
    Notifier() {
    }
	
    virtual void on_change(int new_value) {
	printf("%d\n", new_value);
    }

    virtual void on_switch(bool pressed) {
	printf("switch %s\n", pressed ? "pressed" : "released");
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

    re = new RotaryEncoder(17, 16, 18);
    re->set_notifier(new Notifier());

    while (readline(buf, sizeof(buf)) != NULL) {
	if (buf[0] == '?') {
	    printf("g - get the current value\n");
	    printf("r min max - set the range\n");
	    printf("s value - set the value\n");
#ifdef PI_PICO
	    printf("bootsel\n");
#endif
	} else if (buf[0] == 'g') {
	    printf("%d %d\n", re->get(), re->get_switch());
	} else if (buf[0] == 's') {
	    re->set(atoi(&buf[1]));
	    printf("%d\n", re->get());
	} else if (buf[0] == 'r') {
	    if (sscanf(&buf[1], "%d %d", &min_value, &max_value) == 2) {
		re->set_range(min_value, max_value);
	    } else {
		printf("Missing values: min max\n");
	    }
#ifdef PI_PICO
	} else if (strcmp(buf, "bootsel") == 0) {
            printf("Rebooting into bootloader mode...\n");
            reset_usb_boot(1<<PICO_DEFAULT_LED_PIN, 0);
#endif
	} else if (buf[0] && buf[0] != '\n') {
	    printf("invalid command\n");
	}
    }
}
