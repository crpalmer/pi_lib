#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "time-utils.h"

#include "pi.h"

void
pi_init(void)
{
}

void
pi_init_no_reboot(void)
{
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
