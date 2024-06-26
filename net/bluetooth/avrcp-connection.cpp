#include "pi.h"
#include "avrcp-connection.h"

bool AVRCPConnection::is_established() {
    return cid != 0;
}

void AVRCPConnection::established(uint8_t *packet) {
    cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
    uint8_t status = avrcp_subevent_connection_established_get_status(packet);
    if (status != ERROR_CODE_SUCCESS){
	printf("AVRCP: Connection failed, status 0x%02x\n", status);
	cid = 0;
	return;
    }

    avrcp_subevent_connection_established_get_bd_addr(packet, addr);
    printf("AVRCP: Connected to %s, cid 0x%02x\n", bd_addr_to_str(addr), cid);

    avrcp_target_support_event(cid, AVRCP_NOTIFICATION_EVENT_BATT_STATUS_CHANGED);
    avrcp_target_support_event(cid, AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED);
    avrcp_target_support_event(cid, AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED);
    avrcp_target_support_event(cid, AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED);
    avrcp_target_support_event(cid, AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
    avrcp_target_battery_status_changed(cid, battery_status);

    // query supported events:
    avrcp_controller_get_supported_events(cid);
}

void AVRCPConnection::released(uint8_t *packet) {
    printf("AVRCP: Channel released: cid 0x%02x\n", avrcp_subevent_connection_released_get_avrcp_cid(packet));
    cid = 0;
    notifications_supported_by_target = 0;
}

uint8_t AVRCPConnection::connect(bd_addr_t addr) {
    return avrcp_connect(addr, &cid);
}

uint8_t AVRCPConnection::disconnect() {
    uint8_t status = avrcp_disconnect(cid);
    cid = 0;
    return status;
}

void AVRCPConnection::add_notification(uint8_t event_id) {
    notifications_supported_by_target |= (1 << event_id);
}

void AVRCPConnection::dump_notifications() {
    printf("AVRCP Controller: supported notifications by target:\n");
    for (uint8_t event_id = (uint8_t) AVRCP_NOTIFICATION_EVENT_FIRST_INDEX; event_id < (uint8_t) AVRCP_NOTIFICATION_EVENT_LAST_INDEX; event_id++){
	printf("   - [%s] %s\n", 
	    (notifications_supported_by_target & (1 << event_id)) != 0 ? "X" : " ", 
	    avrcp_notification2str((avrcp_notification_event_id_t) event_id));
    }
    printf("\n\n");
}

void AVRCPConnection::enable_notification(avrcp_notification_event_id_t event_id) {
    avrcp_controller_enable_notification(cid, event_id);
}
