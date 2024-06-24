#include "pi.h"
#include "consoles.h"
#include "bluetooth/a2dp-sink.h"
#include "sbc-decoder.h"

SBCDecoder::SBCDecoder(A2DPSinkHandler *handler) : handler(handler) {
    // Setup the AVDTP endpoint
    a2dp_sink_create_stream_endpoint(AVDTP_AUDIO, AVDTP_CODEC_SBC, sbc_codec_capabilities, sizeof(sbc_codec_capabilities), sbc_codec_configuration, sizeof(sbc_codec_configuration));
}

static void C_handle_pcm_data(int16_t *data, int num_audio_frames, int num_channels, int sample_rate, void *decoder_as_vp) {
    SBCDecoder *decoder = (SBCDecoder *) decoder_as_vp;
    decoder->handle_pcm_data(data, num_audio_frames);
}

void SBCDecoder::handle_pcm_data(int16_t *data, int num_audio_frames) {
    handler->on_pcm_data((uint8_t *) data, num_audio_frames * get_bytes_per_sample() * get_num_channels());
}

void SBCDecoder::media_init() {
    btstack_sbc_decoder_init(&state, mode, C_handle_pcm_data, this);
    handler->on_configure(this);
    audio_stream_started = false;
    media_initialized = true;
}

void SBCDecoder::start() {
    if (! media_initialized) return;
    audio_stream_started = true;
}

void SBCDecoder::pause(void) {
    if (! media_initialized) return;
    audio_stream_started = false;
}

void SBCDecoder::media_close() {
    if (!media_initialized) return;

    media_initialized = 0;
    audio_stream_started = 0;
}

bool SBCDecoder::parse_media_header(uint8_t *packet, int size, int *offset, avdtp_media_packet_header_t *media_header) {
    int media_header_len = 12; // without crc
    int pos = *offset;
    
    if (size - pos < media_header_len){
        consoles_printf("Not enough data to read media packet header, expected %d, received %d\n", media_header_len, size-pos);
        return false;
    }

    media_header->version = packet[pos] & 0x03;
    media_header->padding = get_bit16(packet[pos],2);
    media_header->extension = get_bit16(packet[pos],3);
    media_header->csrc_count = (packet[pos] >> 4) & 0x0F;
    pos++;

    media_header->marker = get_bit16(packet[pos],0);
    media_header->payload_type  = (packet[pos] >> 1) & 0x7F;
    pos++;

    media_header->sequence_number = big_endian_read_16(packet, pos);
    pos+=2;

    media_header->timestamp = big_endian_read_32(packet, pos);
    pos+=4;

    media_header->synchronization_source = big_endian_read_32(packet, pos);
    pos+=4;

    *offset = pos;

    return true;
}

bool SBCDecoder::parse_sbc_header(uint8_t *packet, int size, int *offset, avdtp_sbc_codec_header_t *sbc_header) {
    int sbc_header_len = 12; // without crc
    int pos = *offset;
    
    if (size - pos < sbc_header_len){
        consoles_printf("Not enough data to read SBC header, expected %d, received %d\n", sbc_header_len, size-pos);
        return false;
    }

    sbc_header->fragmentation = get_bit16(packet[pos], 7);
    sbc_header->starting_packet = get_bit16(packet[pos], 6);
    sbc_header->last_packet = get_bit16(packet[pos], 5);
    sbc_header->num_frames = packet[pos] & 0x0f;
    pos++;

    *offset = pos;
    return true;
}

void SBCDecoder::packet_handler(uint8_t seid, uint8_t *packet, uint16_t size) {
    UNUSED(seid);
    int pos = 0;
     
    avdtp_media_packet_header_t media_header;
    avdtp_sbc_codec_header_t sbc_header;

    if (! parse_media_header(packet, size, &pos, &media_header)) return;
    if (! parse_sbc_header(packet, size, &pos, &sbc_header)) return;

    int packet_length = size-pos;
    uint8_t *packet_begin = packet+pos;

    btstack_sbc_decoder_process_data(&state, 0, packet_begin, packet_length);
}

void SBCDecoder::dump_state() {
    printf("    - num_channels: %d\n", num_channels);
    printf("    - sampling_frequency: %d\n", sampling_frequency);
    printf("    - channel_mode: %d\n", channel_mode);
    printf("    - block_length: %d\n", block_length);
    printf("    - subbands: %d\n", subbands);
    printf("    - allocation_method: %d\n", allocation_method);
    printf("    - bitpool_value [%d, %d] \n", min_bitpool_value, max_bitpool_value);
    printf("\n");
}

void SBCDecoder::receive_configuration(uint8_t *packet) {
    reconfigure = a2dp_subevent_signaling_media_codec_sbc_configuration_get_reconfigure(packet);
    num_channels = a2dp_subevent_signaling_media_codec_sbc_configuration_get_num_channels(packet);
    sampling_frequency = a2dp_subevent_signaling_media_codec_sbc_configuration_get_sampling_frequency(packet);
    block_length = a2dp_subevent_signaling_media_codec_sbc_configuration_get_block_length(packet);
    subbands = a2dp_subevent_signaling_media_codec_sbc_configuration_get_subbands(packet);
    min_bitpool_value = a2dp_subevent_signaling_media_codec_sbc_configuration_get_min_bitpool_value(packet);
    max_bitpool_value = a2dp_subevent_signaling_media_codec_sbc_configuration_get_max_bitpool_value(packet);
    
    // Adapt Bluetooth spec definition to SBC Encoder expected input
    int bt_spec_allocation_method = a2dp_subevent_signaling_media_codec_sbc_configuration_get_allocation_method(packet);
    allocation_method = (btstack_sbc_allocation_method_t) (bt_spec_allocation_method - 1);
   
    switch (a2dp_subevent_signaling_media_codec_sbc_configuration_get_channel_mode(packet)){
    case AVDTP_CHANNEL_MODE_JOINT_STEREO:
	channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
	break;
    case AVDTP_CHANNEL_MODE_STEREO:
	channel_mode = SBC_CHANNEL_MODE_STEREO;
	break;
    case AVDTP_CHANNEL_MODE_DUAL_CHANNEL:
	channel_mode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
	break;
    case AVDTP_CHANNEL_MODE_MONO:
	channel_mode = SBC_CHANNEL_MODE_MONO;
	break;
    default:
	btstack_assert(false);
	break;
    }
}

void SBCDecoder::stream_started() {
    if (reconfigure){
	media_close();
    }
    // prepare media processing
    media_init();
}
