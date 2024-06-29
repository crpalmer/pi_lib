#ifndef __AVRCP_CONNECTION_H__
#define __AVRCP_CONNECTION_H__

#include "btstack.h"

class AVRCPConnection {
public:
    bool is_established();

    void established(uint8_t *packet);
    void released(uint8_t *packet);

    uint8_t connect(bd_addr_t addr);
    uint8_t disconnect();

    void add_notification(uint8_t event_id);
    void dump_notifications();

    void enable_notification(avrcp_notification_event_id_t event_id);

    uint16_t get_cid() { printf("cid = %d\n", cid); return cid; }

private:
    bd_addr_t addr;
    uint16_t  cid = 0;

    uint16_t  notifications_supported_by_target = 0;
    avrcp_battery_status_t battery_status = AVRCP_BATTERY_STATUS_WARNING;
};

#endif
