#define USE_FREERTOS 1

#include <cstdio>
#include "FreeRTOS.h"
#include "task.h"

#include "pi.h"
#include "util.h"

typedef struct {
    const char *string;
    unsigned ms;
} print_task_t;

void print_task(void *task_as_vp)
{
    print_task_t *t = (print_task_t *) task_as_vp;

    while (1) {
	ms_sleep(t->ms);
	printf("%s\n", t->string);
    }
}

int main(int argc, char **argv)
{
    pi_init();

    print_task_t hi = {  "hi", 1000 };
    print_task_t bye = { "bye", 450 };

    xTaskCreate(print_task, "Hi", 1024, &hi, 1, NULL);
    xTaskCreate(print_task, "Bye", 1024, &bye, 1, NULL);

    vTaskStartScheduler();
}

