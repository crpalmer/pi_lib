#include <stdio.h>
#include <usb.h>
#include "pi-usb.h"

void
pi_usb_init(void)
{
   usb_init();
   usb_find_busses();
   usb_find_devices();
}

struct usb_device *
pi_usb_device(unsigned vendor_id, unsigned product_id)
{
    struct usb_bus *bus;

    bus = usb_get_busses();

    while (bus != NULL) {
	struct usb_device *dev;;

	for (dev = bus->devices; dev; dev = dev->next) {
	    if (dev->descriptor.idVendor == vendor_id &&
		dev->descriptor.idProduct == product_id)
	    {
		return dev;
	    }
	}

	bus = bus->next;
    }

    return NULL;
}
