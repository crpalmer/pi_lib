#ifndef __PI_H__
#define __PI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void pi_init(void);
void pi_init_no_reboot();

char *pi_readline(char *buf, size_t buflen);

#ifdef __cplusplus
};
#endif

#endif
