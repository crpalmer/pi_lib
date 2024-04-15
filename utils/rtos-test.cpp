#include <stdio.h>
#include <string.h>
#include "pi.h"
#include "pi-threads.h"
#include "time-utils.h"

void hello(void *arg_unused)
{
    for (;;) {
	printf("hello\n");
	ms_sleep(2000);
    }
}

void world(void *arg_unused)
{
    ms_sleep(1000);
    for (;;) {
	printf("World\n");
	ms_sleep(2000);
    }
}

void readline_task(void *arg_unused) {
    static char cmd[128];
    while (pi_readline(cmd, sizeof(cmd)) != NULL) {
	if (strcmp(cmd, "bootsel") == 0) {
	    pi_reboot_bootloader();
	} else if (strcmp(cmd, "dump-state") == 0) {
	    pi_threads_dump_state();
	} else if (strcmp(cmd, "?") == 0) {
	    fprintf(stderr, "Available commands:\n");
	    fprintf(stderr, "  bootsel\n");
	    fprintf(stderr, "  dump-state\n");
	} else {
	    fprintf(stderr, "invalid command\n");
	}
    }
}

int main() {
    pi_threads_init();
    pi_thread_create_anonymous(hello, NULL);
    pi_thread_create_anonymous(world, NULL);
    pi_thread_create_anonymous(readline_task, NULL);
    pi_threads_start_and_wait();
}
