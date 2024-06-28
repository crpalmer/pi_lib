#ifndef __SBC_DECODER_H__
#define __SBC_DECODER_H__

#include "btstack.h"
#include "sbc-configuration.h"

class SBCDecoder {
public:
    SBCDecoder(class A2DPSinkHandler *handler);

    void dump_state() { configuration->dump(); }
    void receive_configuration(uint8_t *packet) { configuration->receive(packet); }

    void media_init();
    void media_close();
    void start();
    void pause();

    void dump_configuration();
    void stream_started();

    void packet_handler(uint8_t seid, uint8_t *packet, uint16_t size);
    void handle_pcm_data(int16_t *data, int num_audio_frames);

private:
    bool parse_media_header(uint8_t *packet, int size, int *offset, avdtp_media_packet_header_t *media_header);
    bool parse_sbc_header(uint8_t *packet, int size, int *offset, avdtp_sbc_codec_header_t *sbc_header);

private:
    class A2DPSinkHandler *handler;
    SBCConfiguration *configuration;

    bool media_initialized = false;
    bool audio_stream_started = false;

    btstack_sbc_decoder_state_t state;
    btstack_sbc_mode_t mode = SBC_MODE_STANDARD;

    uint8_t sbc_codec_configuration[4];
    const uint8_t sbc_codec_capabilities[4] = {
        0xFF,//(AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
        0xFF,//(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) | AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
        2, 53
    };
};

#endif
