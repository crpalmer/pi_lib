#include <stdio.h>
#include "pi.h"
#include "pi-threads.h"

void vApplicationStackOverflowHook(TaskHandle_t task, char *name )
{
   fprintf(stderr, "%s: stack overflow!\n", name);
   pi_reboot_bootloader();
}

void vApplicationMallocFailedHook(TaskHandle_t task, char *name)
{
   fprintf(stderr, "%s: malloc failed!\n", name);
   pi_reboot_bootloader();
}
