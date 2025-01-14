#ifndef __A2DP_SINK_H__
#define __A2DP_SINK_H__

#include "audio.h"

class A2DPSinkHandler {
public:
    virtual void on_configure(AudioConfig *config) = 0;
    virtual void on_pcm_data(uint8_t *data, size_t n) = 0;
};

class A2DPSink : public A2DPSinkHandler {
public:
    A2DPSink();

    void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t event_size);
    void media_packet_handler(uint8_t seid, uint8_t *packet, uint16_t size);

    virtual void on_configure(AudioConfig *config) override { }
    virtual void on_pcm_data(uint8_t *data, size_t n) override { }

private:
    uint8_t service_buffer[150];
    uint8_t sdp_avrcp_target_service_buffer[150];
    uint8_t sdp_avrcp_controller_service_buffer[200];

    class AVRCP *avrcp;
    class A2DPSinkConnection *connection;
    class SBCDecoder *decoder;
};

#endif
