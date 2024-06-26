#ifndef __AVRCP_H__
#define __AVRCP_H__

#include "avrcp-connection.h"

typedef enum {
    AVRCP_BUTTON_PLAY,
    AVRCP_BUTTON_PAUSE,
    AVRCP_BUTTON_STOP,
    AVRCP_BUTTON_VOLUME_UP,
    AVRCP_BUTTON_VOLUME_DOWN,
} avrcp_button_t;

class AVRCP {
public:
    AVRCP();

    void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void controller_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
    void target_packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

    uint8_t connect(bd_addr_t addr);
    uint8_t disconnect();

    uint8_t volume_up();
    uint8_t volume_down();
    uint8_t set_volume(int volume);

    uint8_t set_now_playing_info(const avrcp_track_t *track, uint16_t n_tracks);
    uint8_t set_playback_status(avrcp_playback_status_t status);

    virtual void on_volume_changed(uint8_t volume) { }
    virtual void on_button_pressed(avrcp_button_t button) { }

private:
    AVRCPConnection *connection;
    int volume_percentage = 0;
};

#endif
