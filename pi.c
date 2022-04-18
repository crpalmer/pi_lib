#include <stdio.h>
#include <stdlib.h>

#include "pi.h"

void
pi_init(void)
{
#ifdef PI_PICO
    stdio_init_all();
#else
#endif
}
