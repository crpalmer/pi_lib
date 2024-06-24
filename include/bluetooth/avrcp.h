#ifndef __AVRCP_H__
#define __AVRCP_H__

class AVRCP {
public:
    AVRCP();

    void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

    virtual void on_volume_changed(uint8_t volume) { }

private:
    class AVRCPConnection *connection;

    int volume_percentage = 0;
};

#endif
