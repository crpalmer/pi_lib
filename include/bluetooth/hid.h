#ifndef __HID_H__
#define __HID_H__

static const uint16_t hid_subclass_mouse = 0x2580;

class HID {
public:
    HID(const char *name, uint8_t *hid_descriptor, uint16_t hid_descriptor_len, uint16_t subclass, bool hid_virtual_cable = false, bool hid_remote_wake = true, bool hid_reconnect_initiate = true, bool hid_normally_connectable = true);

    bool is_connected() { return cid != 0; }

    void send_report(uint8_t *data, int n_data);
    virtual void can_send_now() = 0;

    void request_can_send_now();

    static void connected(class HID *hid, uint16_t cid);
    static void can_send_now(class HID *hid);
    static void disconnected(class HID *hid);

protected:
    uint16_t cid = 0;
};

void hid_init();

#endif
