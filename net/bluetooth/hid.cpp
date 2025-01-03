#include "pi.h"
#include "btstack.h"
#include "bluetooth/hid.h"

class HID *hid = NULL;

static void hid_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size) {
    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_HID_META) return;

    switch (hci_event_hid_meta_get_subevent_code(packet)) {
    case HID_SUBEVENT_CONNECTION_OPENED:
	printf("hid connection opened: status %d\n", hid_subevent_connection_opened_get_status(packet));
	if (hid_subevent_connection_opened_get_status(packet) != ERROR_CODE_SUCCESS) return;
	if (hid) HID::connected(hid, hid_subevent_connection_opened_get_hid_cid(packet));
	break;
    case HID_SUBEVENT_CONNECTION_CLOSED:
	printf("hid connection closed\n");
 	if (hid) HID::disconnected(hid);
	break;
    case HID_SUBEVENT_CAN_SEND_NOW:
 	if (hid) HID::can_send_now(hid);
	break;
    }
}

HID::HID(const char *name, uint8_t *hid_descriptor, uint16_t hid_descriptor_len, uint16_t subclass, bool hid_virtual_cable, bool hid_remote_wake, bool hid_reconnect_initiate, bool hid_normally_connectable) {
    const uint8_t hid_boot_device = 0;
    hid_sdp_record_t hid_params = {
        subclass, 
        33, // hid country code 33 US
        hid_virtual_cable, hid_remote_wake, 
        hid_reconnect_initiate, hid_normally_connectable,
        hid_boot_device, // hid_boot_device, 
        0xFFFF, 0xFFFF, 3200,
        hid_descriptor, hid_descriptor_len,
        name
    };

    static uint8_t hid_service_buffer[270];

    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
    hid_create_sdp_record(hid_service_buffer, sdp_create_service_record_handle(), &hid_params);
    btstack_assert(de_get_len(hid_service_buffer) <= sizeof(hid_service_buffer));
    sdp_register_service(hid_service_buffer);

#if 0
    static btstack_packet_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &hid_hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
#endif

    hid_device_init(hid_boot_device, hid_descriptor_len, hid_descriptor);
    hid_device_register_packet_handler(&hid_packet_handler);

    // TODO how to differentiate different devices
    hid = this;
}

void HID::request_can_send_now() {
    hid_device_request_can_send_now_event(cid);
}

void HID::connected(class HID *hid, uint16_t cid) {
    hid->cid = cid;
    hid->on_connect();
}

void HID::can_send_now(class HID *hid) {
    hid->can_send_now();
}

void HID::disconnected(class HID *hid) {
    hid->cid = 0;
    hid->on_disconnect();
}

void HID::send_report(uint8_t *data, int n_data) {
    hid_device_send_interrupt_message(cid, data, n_data);
}

void hid_init() {
    static btstack_packet_callback_registration_t hci_event_callback_registration;

    hci_event_callback_registration.callback = &hid_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
}
