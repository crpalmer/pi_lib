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
 
void bluetooth_init(void) {
    net_platform_init();
 
    // inform about BTstack state 
    static btstack_packet_callback_registration_t hci_event_callback_registration;
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // init protocols
    l2cap_init();
    sdp_init();

    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();
}
