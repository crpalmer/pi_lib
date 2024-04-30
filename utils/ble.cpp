#include <stdio.h>
#include "ble.h"
#include "pi.h"

static char buffer[1024];

int
main()
{
    pi_init();

    BLE *ble = new BLE();
    while (1) {
	char name[1000];

	ble->accept_connection();
	printf("Connected @ %d\n", ble->get_baud());
	ble->set_ble_name("ble");
	ble->get_ble_name(name);
	printf("Name is %s\n", name);
	while (ble->is_connected()) {
	    if (ble->readline(buffer, sizeof(buffer), 1000)) {
		ble->puts(buffer);
		ble->putc('\n');
	    }
	}
	printf("Disconnected.\n");
    }
    return 0;
}


