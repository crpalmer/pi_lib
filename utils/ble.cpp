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
	ble->accept_connection();
	while (ble->is_connected()) {
	    char c = ble->getc();
printf("%c", c);
	    ble->putc(c);
	}
    }
    return 0;
}


