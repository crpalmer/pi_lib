#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pi-gpio.h"
#include "time-utils.h"

#include "pi.h"

void
pi_init(void)
{
    pi_gpio_init();
}

void
pi_abort(void)
{
    fprintf(stderr, "pi: fatal error, exiting.\n");
    exit(0);
}

void
pi_reboot_bootloader(void)
{
    fprintf(stderr, "pi: no bootloader available.  Exiting instead.\n");
    exit(0);
}

char *
pi_readline(char *buf, size_t buf_len)
{
    char *ret = fgets(buf, buf_len, stdin);
    if (ret) {
	size_t len = strlen(buf);
	while (len > 0 && buf[len-1] == '\n') len--;
	buf[len] = '\0';
    }
    return ret;
}
