#include <stdio.h>
#include "pi.h"
#include "mem.h"
#include "pi-threads.h"

void vApplicationStackOverflowHook(TaskHandle_t task, char *name )
{
   fprintf(stderr, "%s: stack overflow!\n", name);
   pi_abort();
}

void vApplicationMallocFailedHook()
{
   fprintf(stderr, "malloc failed!\n");
   pi_reboot_bootloader();
}
