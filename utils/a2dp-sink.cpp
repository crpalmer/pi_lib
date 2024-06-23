#include "pi.h"
#include "bluetooth/bluetooth.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wifi.h"

int btstack_main(int argc, const char * argv[]);

void thread_main(int argc, char **argv) {
    ms_sleep(1000);

#ifdef WITH_WIFI
    wifi_init(CYW43_HOST_NAME);
    wifi_wait_for_connection();
#endif

    bluetooth_init();

    btstack_main(0, NULL);
    while (1) ms_sleep(1000000);
}

int main(int argc, char **argv) {
    pi_init_with_threads(thread_main, argc, argv);
}
