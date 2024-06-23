#include "pi.h"
#include "pi-threads.h"

int btstack_main(int argc, const char * argv[]);

void thread_main(int argc, char **argv) {
    btstack_main(0, NULL);
}

int main(int argc, char **argv) {
    pi_init_with_threads(thread_main, argc, argv);
}
