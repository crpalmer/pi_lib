#include <stdio.h>
#include "ble.h"
#include "pi.h"
#include "util.h"

int
main()
{
    pi_init();

ms_sleep(5000);

    BLE *ble = new BLE();
    while (1) {
	char name[1000];

	ble->accept_connection();
	printf("Connected @ %d\n", ble->get_baud());
	ble->get_ble_name(name);
	printf("Name is %s\n", name);
	ble->set_ble_name("utils-ble");
	ble->get_ble_name(name);
	printf("Name is %s\n", name);
	while (ble->is_connected()) {
	    char c = ble->getc();
	    ble->putc(c);
	}
    }
    return 0;
}


