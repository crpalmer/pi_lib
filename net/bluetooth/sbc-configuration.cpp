#include "pi.h"
#include "consoles.h"
#include "bluetooth/a2dp-sink.h"
#include "sbc-decoder.h"

void SBCConfiguration::dump() {
    printf("    - num_channels: %d\n", num_channels);
    printf("    - sampling_frequency: %d\n", sampling_frequency);
    printf("    - channel_mode: %d\n", channel_mode);
    printf("    - block_length: %d\n", block_length);
    printf("    - subbands: %d\n", subbands);
    printf("    - allocation_method: %d\n", allocation_method);
    printf("    - bitpool_value [%d, %d] \n", min_bitpool_value, max_bitpool_value);
    printf("\n");
}

void SBCConfiguration::receive(uint8_t *packet) {
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

void SBCConfiguration::encoder_init(btstack_sbc_encoder_state_t *state) {
    btstack_sbc_encoder_init(state, SBC_MODE_STANDARD, block_length, subbands, allocation_method, sampling_frequency, max_bitpool_value, channel_mode);
}

