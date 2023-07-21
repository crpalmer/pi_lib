#ifndef __PI_H__
#define __PI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#ifdef PI_PICO
#include "pico/stdlib.h"
#else
#endif

void pi_init_no_reboot();

#ifdef USE_FREERTOS
inline void pi_init() { pi_init_no_reboot(); }
#else
void pi_init(void);
#endif

#ifdef PI_PICO
void pico_readline(char *buf, size_t buflen);
void pico_readline_echo(char *buf, size_t buflen, bool echo);
#endif

#ifdef __cplusplus
};
#endif

#endif
