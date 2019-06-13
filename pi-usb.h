#ifndef __PI_USB_H__
#define __PI_USB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <usb.h>

void
pi_usb_init(void);

struct usb_device *
pi_usb_device(unsigned vendor_id, unsigned product_id);

#ifdef __cplusplus
};
#endif

#endif
