#ifndef __SBC_CONFIGURATION_H__
#define __SBC_CONFIGURATION_H__

#include "audio.h"
#include "btstack.h"

class SBCConfiguration : public AudioConfig {
public:
    void dump();
    void receive(uint8_t *packet);

    bool needs_reconfiguration() { return reconfigure; }
    void encoder_init(btstack_sbc_encoder_state_t *state);

    int get_num_channels() override { return num_channels; }
    int get_rate() override { return sampling_frequency; }
    int get_bytes_per_sample() override { return 2; }

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
};

#endif
