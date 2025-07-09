#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "mqtt.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wifi.h"

static void
threads_main(int argc, char **argv)
{
    int i = 0;

    wifi_init(CYW43_HOST_NAME);
    wifi_wait_for_connection();

    MQTT *mqtt = new MQTT();
    mqtt->start();

    while (1) {
	char str[128];
	ms_sleep(10*1000);
	sprintf(str, "ping %d", i++);
	mqtt->publish("mqtt-test", str);
    }
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
