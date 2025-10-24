#include "pi.h"
#include <pico/stdlib.h>
#include <hardware/watchdog.h>
#include <sys/time.h>
#include "pi-gpio.h"
#include "time-utils.h"

#include "call-every.h"
#include "hardware/adc.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include <pico/bootrom.h>
#ifdef HAVE_RTC
#include "hardware/rtc.h"
#endif

static int (*pre_set_irq_fn)() = NULL;
static void (*post_set_irq_fn)(int saved) = NULL;

int pico_pre_set_irq() {
    if (pre_set_irq_fn) return pre_set_irq_fn();
    return 0;
}

void pico_post_set_irq(int saved) {
    if (post_set_irq_fn) post_set_irq_fn(saved);
}

void pico_set_irq_hook_functions(int (*pre)(), void (*post)(int)) {
    pre_set_irq_fn = pre;
    post_set_irq_fn = post;
}

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

void
pi_reboot() {
    printf("Using watchdog to reboot.\n");
    watchdog_reboot(0, 0, 0);
}

void
pi_reboot_bootloader()
{
    fprintf(stderr, "rebooting to bootsel mode\n");
    reset_usb_boot(0, 0);
}

void
pi_abort()
{
    __breakpoint();
}

void
pi_init(void)
{
    stdio_init_all();
    adc_init();
    pi_gpio_init();
    setenv("TZ", "EST+5EDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();
}

char *
pi_readline(char *buf, size_t buf_len)
{
    int n_buf = 0;

    while (1) {
	char c;

	if ((c = getchar()) > 0) {
	    putchar(c);
	    if (c == '\r' || c == '\n') {
		buf[n_buf] = '\0';
		return buf;
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

void
pico_set_rtc(time_t secs)
{
#ifdef HAVE_RTC
    struct tm tm;
    datetime_t dt;

    gmtime_r(&secs, &tm);
    dt.year  = tm.tm_year;
    dt.month = tm.tm_mon + 1;
    dt.day   = tm.tm_mday;
    dt.dotw  = tm.tm_wday;
    dt.hour  = tm.tm_hour;
    dt.min   = tm.tm_min;
    dt.sec   = tm.tm_sec;

    rtc_set_datetime(&dt);

    struct timeval tv = {.tv_sec = secs, };
    settimeofday(&tv, NULL);
#else
    assert(0);
#endif
}
