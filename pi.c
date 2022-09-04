#include <stdio.h>
#include <stdlib.h>
#include "time-utils.h"

#include "pi.h"

#ifdef PI_PICO

#include "call-every.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include <pico/bootrom.h>

bool __no_inline_not_in_flash_func(get_bootsel_button)() {
    const uint CS_PIN_INDEX = 1;

    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i);

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
    bool button_state = !(sio_hw->gpio_hi_in & (1u << CS_PIN_INDEX));

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

static call_every_t *call_every;

static int pushed = 0;

static void
reboot_on_button_press(void *unused)
{
    if (get_bootsel_button()) {
	if (pushed) {
	     printf("Button long pressed, rebooting to bootsel mode\n");
	     reset_usb_boot(1<<PICO_DEFAULT_LED_PIN, 0);
	} else {
	     pushed = 1;
	}
    } else {
	pushed = 0;
    }
}

#endif

void
pi_init(void)
{
#ifdef PI_PICO
    pi_init_no_reboot();
    call_every = call_every_new(1000, reboot_on_button_press, NULL);
    call_every_start(call_every);
#endif
}

void
pi_init_no_reboot(void)
{
#ifdef PI_PICO
    stdio_init_all();
#endif
}

#ifdef PI_PICO

void
pico_readline(char *buf, size_t buf_len)
{
    pico_readline_echo(buf, buf_len, false);
}

void
pico_readline_echo(char *buf, size_t buf_len, bool echo)
{
    int n_buf = 0;

    while (1) {
	char c;

	if ((c = getchar()) > 0) {
	    if (echo) putchar(c);
	    if (c == '\r' || c == '\n') {
		buf[n_buf] = '\0';
		return;
	    } else if (c == 8) {
		if (n_buf) {
		    n_buf--;
		    putchar(' ');
		    putchar(8);
		}
	    } else if (n_buf < buf_len-1) {
		buf[n_buf++] = c;
	    }
	}
    }
}

#endif
