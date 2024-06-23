#include "pi.h"
#include "pi-threads.h"
#include "time-utils.h"

int btstack_main(int argc, const char * argv[]);

void thread_main(int argc, char **argv) {
    ms_sleep(1000);
    btstack_main(0, NULL);
    while (1) ms_sleep(1000000);
}

int main(int argc, char **argv) {
    pi_init_with_threads(thread_main, argc, argv);
}
