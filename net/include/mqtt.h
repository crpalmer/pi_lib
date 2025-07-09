#ifndef __MQTT_H__
#define __MQTT_H__

#include "pi-threads.h"

struct mqtt_internal_state;

class MQTT : public PiThread {
public:
    MQTT(const char *name = "mqtt");
    ~MQTT();

    void main() override;

    void publish(const char *topic, const char *message);
    void alert(const char *topic, const char *message);

private:
    struct mqtt_internal_state *state;
    enum { DISCONNECTED, CONNECTING, CONNECTED } status = DISCONNECTED;

    void callback(struct mg_connection *c, int ev, void *ev_data);
    void reconnect();

    static void callback_proxy(struct mg_connection *c, int ev, void *ev_data);
    static void reconnect_proxy(void *data);
};

#endif
