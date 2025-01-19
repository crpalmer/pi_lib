#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

void bluetooth_init();

static const uint8_t bt_major_service_class_capturing = 0x08;
static const uint8_t bt_major_service_class_audio = 0x20;

static const uint8_t bt_major_device_class_audio = 0x04;
static const uint8_t bt_minor_device_class_headset = 0x04;
static const uint8_t bt_minor_device_class_hfp = 0x08;
static const uint8_t bt_minor_device_class_microphone = 0x10;
static const uint8_t bt_minor_device_class_loudspeaker = 0x14;

static const uint8_t bt_major_device_class_peripheral = 0x05;
static const uint8_t bt_minor_device_class_gamepad = 0x08;

void bluetooth_start(uint32_t service_class, const char *name = "Pico BT");

static inline void bluetooth_start_a2dp_sink(const char *name = "Pico A2DP Sink") {
    bluetooth_start((bt_major_service_class_audio << 16) | (bt_major_device_class_audio << 8) | bt_minor_device_class_headset | bt_minor_device_class_microphone, name);
}

static inline void bluetooth_start_a2dp_source(const char *name = "Pico A2DP Source") {
    bluetooth_start((bt_major_service_class_audio << 16) | (bt_major_device_class_audio << 8) | bt_minor_device_class_hfp, name);
}

static inline void bluetooth_start_gamepad(const char *name = "Pico Gamepad") {
    bluetooth_start((bt_major_device_class_peripheral << 8) | bt_minor_device_class_gamepad, name);
}

#endif
