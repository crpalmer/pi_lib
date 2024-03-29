#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "time-utils.h"
#include "wb.h"

static char buf[1024];
static pthread_t watcher;
static volatile int watcher_enabled;

#define WATCHER_DEBOUNCE_MS 5

static void
print_inputs(unsigned value)
{

    int pin;

    for (pin = 7; pin >= 0; pin--) {
	printf("%d", (value & (1<<pin)) != 0);
    }
}

static void *
watcher_main(void *unused)
{
    unsigned last = wb_get_all_with_debounce(WATCHER_DEBOUNCE_MS);
    int was_enabled = 0;
    struct timespec at;

    while (true) {
	if (watcher_enabled)  {
	    unsigned cur;
	    if (! was_enabled) {
		was_enabled = 1;
		nano_gettime(&at);
		last = cur;
	    } else {
	        cur = wb_get_all_with_debounce(WATCHER_DEBOUNCE_MS);
		if (cur != last) {
		    print_inputs(cur);
		    printf(" %d\n", nano_elapsed_ms_now(&at));
		    last = cur;
		    nano_gettime(&at);
		}
	    }
	}
    }
    return NULL;
}

int
main(int argc, char **argv)
{
    if (wb_init() < 0) {
	fprintf(stderr, "Failed to initialize wb\n");
	exit(1);
    }

    pthread_create(&watcher, NULL, watcher_main, NULL);

    while (fgets(buf, sizeof(buf), stdin) != NULL && ! feof(stdin)) {
	int bank, pin, value;

	if (buf[0] == 'g') {
	    if (sscanf(&buf[1], "%d", &pin) == 1) {
		printf("%d = %d\n", pin, wb_get(pin));
	    } else {
		print_inputs(wb_get_all());
		printf("\n");
	    }
	} else if (buf[0] == 'G') {
	    int last = -1;

	    while (true) {
		int cur = wb_get_all();
		if (cur != last) {
		    int pin;

		    last = cur;
		    for (pin = 7; pin >= 0; pin--) {
			printf("%d", (cur & (1<<pin)) != 0);
		    }
		    printf("\n");
		}
	    }
	} else if (buf[0] == '=' || buf[0] == '+' || buf[0] == '-') {
	    if (sscanf(&buf[1], "%d", &pin) == 1) {
		wb_set_pull_up(pin, buf[0] == '=' ? WB_PULL_UP_NONE : buf[0] == '+' ? WB_PULL_UP_UP : WB_PULL_UP_DOWN);
	    } else {
		goto usage;
	    }
	} else if (buf[0] == 's') {
	    if (sscanf(&buf[1], "%d %d %d", &bank, &pin, &value) == 3) {
		wb_set(bank, pin, value);
	    } else {
		goto usage;
	    }
	} else if (buf[0] == 'p') {
	    unsigned freq;
	    float fvalue;
	    if (sscanf(&buf[1], "%d %d %u %f", &bank, &pin, &freq, &fvalue) == 4) {
		wb_pwm_freq(bank, pin, freq, fvalue/100.0);
	    } else if (sscanf(&buf[1], "%d %d %f", &bank, &pin, &fvalue) == 3) {
		wb_pwm(bank, pin, fvalue/100.0);
	    } else {
		goto usage;
	    }
	} else if (buf[0] == 'w' || buf[0] == 'W') {
	    unsigned mask, values = buf[0] == 'w' ? WB_PIN_MASK_ALL : 0;
	    if (sscanf(&buf[1], "%d", &pin) == 1) {
		mask = WB_PIN_MASK(pin);
	    } else {
		mask = WB_PIN_MASK_ALL;
	    }
	    printf("wait got pin %d\n", wb_wait_for_pins(mask, values));
	} else if (buf[0] == 'T' || buf[0] == 't') {
	    watcher_enabled = buf[0] == 'T';
	} else {
usage:
	    fprintf(stderr, "g [<pin 1-8>]\np <bank 1/2> <pin 1-8> [<freq>] <duty%%>\ns <bank 1/2> <pin 1-8> <value 0/1>\n- <pin 1-8> set pull down\n+ <pin 1-8> set pull up\n= <pin 1-8> remove pulll up/down\nw [ <pin 1-8> ] wait for a pin to read true\nW [ <pin 1-8> ] wait for a pin to read false\nT enable watcher thread\nt disable watcher thread\n");
	}
    }

    return 0;
}
