#ifndef __SBC_DECODER_H__
#define __SBC_DECODER_H__

#include "btstack.h"

class SBCDecoder {
public:
    SBCDecoder();

    void media_init();
    void media_close();
    void start();
    void pause();

    void dump_state();
    void receive_configuration(uint8_t *packet);
    void stream_started();

    void packet_handler(uint8_t seid, uint8_t *packet, uint16_t size);

private:
    bool parse_media_header(uint8_t *packet, int size, int *offset, avdtp_media_packet_header_t *media_header);
    bool parse_sbc_header(uint8_t *packet, int size, int *offset, avdtp_sbc_codec_header_t *sbc_header);

private:
    bool  reconfigure = false;
    uint8_t  num_channels = 2;
    uint16_t sampling_frequency = 44100;
    uint8_t  block_length = 0;
    uint8_t  subbands = 0;
    uint8_t  min_bitpool_value = 0;
    uint8_t  max_bitpool_value = 0;
    btstack_sbc_channel_mode_t channel_mode;
    btstack_sbc_allocation_method_t allocation_method;

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
