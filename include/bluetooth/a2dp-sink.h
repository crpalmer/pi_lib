#ifndef __A2DP_SINK_H__
#define __A2DP_SINK_H__

class A2DPSink {
public:
    A2DPSink();

    void initialize();
    void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t event_size);
    void media_packet_handler(uint8_t seid, uint8_t *packet, uint16_t size);

private:
    uint8_t service_buffer[150];
    class A2DPSinkConnection *connection;
    class SBCDecoder *decoder;
};

#endif
