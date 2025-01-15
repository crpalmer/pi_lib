#ifndef __PI_H__
#define __PI_H__

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "file.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <time.h>

void pi_init(void);
void pi_init_no_reboot();

char *pi_readline(char *buf, size_t buflen);

void pi_abort();
void pi_reboot();
void pi_reboot_bootloader();

#ifdef __cplusplus
};
#endif

#include "pi-platform.h"

#endif
