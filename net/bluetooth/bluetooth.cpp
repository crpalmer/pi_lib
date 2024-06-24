#include "pi.h"
#include "net.h"
#include "btstack.h"
#include "bluetooth/bluetooth.h"

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    if (packet_type != HCI_EVENT_PACKET) return;

    switch(hci_event_packet_get_type(packet)){ 
    case BTSTACK_EVENT_STATE: {
	if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        bd_addr_t local_addr; 
	gap_local_bd_addr(local_addr);
	printf("BTstack up and running on %s.\n", bd_addr_to_str(local_addr));
	break;
    }
    default:
	break;
    }
}
 
void bluetooth_init() {
    net_platform_init();
 
    // inform about BTstack state 
    static btstack_packet_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // When using freertos this seems to deadlock when trying to write a connection
    // key to flash memory.  Turning off bondable_mode seems to fix that.
    gap_set_bondable_mode(false);

    // init protocols
    l2cap_init();
    sdp_init();

    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();

    // - Create and register Device ID (PnP) service record
    static uint8_t device_id_sdp_service_buffer[100];
    memset(device_id_sdp_service_buffer, 0, sizeof(device_id_sdp_service_buffer));
    device_id_create_sdp_record(device_id_sdp_service_buffer, sdp_create_service_record_handle(), DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH, BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    sdp_register_service(device_id_sdp_service_buffer);
}

void bluetooth_start(uint32_t service_class, const char *name) {
    gap_set_local_name(name);
    gap_discoverable_control(1);
    gap_set_class_of_device(service_class);
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE);
    gap_set_allow_role_switch(true);

    hci_power_control(HCI_POWER_ON);
}

