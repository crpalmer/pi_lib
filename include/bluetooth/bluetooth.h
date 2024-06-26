#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

void bluetooth_init();

void bluetooth_start(uint32_t service_class, const char *name = "Pico BT");

static inline void bluetooth_start_a2dp_sink(const char *name = "Pico A2DP Sink") {
    bluetooth_start(0x200414, name); // Service Class: Audio, Major Device Class: Audio, Minor: Loudspeaker
}

static inline void bluetooth_start_a2dp_source(const char *name = "Pico A2DP Source") {
    bluetooth_start(0x200408, name); // Service Class: Audio, Major Device Class: Audio, Minor: Loudspeaker
}

#endif
