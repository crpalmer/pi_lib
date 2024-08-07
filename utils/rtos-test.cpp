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

void threads_main(int argc, char **argv) {
    pi_thread_create("hello", hello, NULL);
    pi_thread_create("world", world, NULL);
    pi_thread_create("readline", readline_task, NULL);
}

int main(int argc, char **argv) {
    pi_init_with_threads(threads_main, argc, argv);
}
