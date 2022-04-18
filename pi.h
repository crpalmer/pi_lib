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

void pi_init(void);

#ifdef __cplusplus
};
#endif

#endif
