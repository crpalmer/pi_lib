#include <stdio.h>
#include <usb.h>
#include <fcntl.h>
#include "pi.h"
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

#define UEVENT_PRODUCT "PRODUCT="

int
pi_usb_open_tty(unsigned vendor_id, unsigned product_id)
{
     char buf[128];
     char fname[1024];

     buf[sizeof(buf)-1] = '\0';

     for (int n = 0; n < 10; n++) {
	FILE *f;
        int fd;
	unsigned vendor, product;

	sprintf(fname, "/sys/class/tty/ttyACM%d/device/uevent", n);
	if ((f = file_open_read(fname)) == NULL) continue;
	while (fgets(buf, sizeof(buf)-1, f) != NULL) {
	    if (sscanf(buf, UEVENT_PRODUCT"%x/%x/", &vendor, &product) != 2) continue;
	    if ((vendor_id && vendor != vendor_id) ||
		(product_id && product != product_id)) {
		continue;
	    }

	    sprintf(fname, "/dev/ttyACM%d", n);
	    if ((fd = open(fname, O_RDWR)) >= 0) {
		file_close(f);
		fprintf(stderr, "Opened tty: %s\n", fname);
		return fd;
	    } else {
		perror(fname);
	    }
	}
     }

     return -1;
}
