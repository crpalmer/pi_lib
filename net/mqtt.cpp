#include "pi.h"
#include "mem.h"
#include "mqtt.h"
#define NOT_IN_MONGOOSE_C
#include "mongoose.h"

#include "time-utils.h"

#define URL "mqtt://165.22.182.89:1883"

class MQTTAction {
public:
    virtual ~MQTTAction() {}
    virtual void act(mg_connection *connection) = 0;
};

class MQTTPublish : public MQTTAction {
public:
    MQTTPublish(const char *topic, const char *msg) : topic(topic), msg(msg) { }
    void act(mg_connection *connection) override {
	if (! connection) {
	    printf("publish %s | %s failed, not connected\n", topic, msg);
	} else {
	    struct mg_mqtt_opts pub_opts = {
		.topic = mg_str(topic),
		.message = mg_str(msg),
		.qos = 1,
		.retain = 0,
	    };

	    mg_mqtt_pub(connection, &pub_opts);
	}
    }

private:
    const char *topic, *msg;
};

struct mqtt_internal_state {
    mg_mgr mgr;
    mg_connection *connection;
};

MQTT::MQTT(const char *name) : PiThread(name) {
    state = (struct mqtt_internal_state *) fatal_malloc(sizeof(*state));
    mg_mgr_init(&state->mgr);
    mg_wakeup_init(&state->mgr);
    state->connection = NULL;
    mg_log_set(MG_LL_VERBOSE);
}

MQTT::~MQTT() {
    if (state->connection) mg_mqtt_disconnect(state->connection, NULL);
}

void MQTT::callback(struct mg_connection *c, int ev, void *ev_data) {
    switch (ev) {
    case MG_EV_MQTT_OPEN:
	printf("mqtt: connected\n");
	break;
    case MG_EV_ERROR:
	MG_ERROR(("%lu ERROR %s", c->id, (char *) ev_data));
	break;
    case MG_EV_CLOSE:
	printf("mqtt: closed\n");
	status = DISCONNECTED;
	state->connection = NULL;
	break;
    case MG_EV_WAKEUP:
	mg_str *ev_data_as_str = (mg_str *) ev_data;
	MQTTAction *action = *(MQTTAction **) ev_data_as_str->buf;
	action->act(c);
	delete action;
	break;
    }
}

void MQTT::callback_proxy(struct mg_connection *c, int ev, void *ev_data) {
    ((class MQTT *) c->fn_data)->callback(c, ev, ev_data);
}

void MQTT::publish(const char *topic, const char *msg) {
    MQTTAction *action = new MQTTPublish(topic, msg);
    mg_wakeup(&state->mgr, state->connection->id, &action, sizeof(action));
}

void MQTT::alert(const char *topic, const char *msg) {
    char full_topic[strlen(topic) + 10];
    sprintf(full_topic, "alerts/%s", topic);
    publish(full_topic, msg);
};

void MQTT::reconnect() {
    if (! state->connection) {
	struct mg_mqtt_opts opts = {
	    .topic = mg_str("topic"),
	    .message = mg_str("bye"),
	    .qos = 1,
	    .version = 4,
	    .clean = true,
	};
	state->connection = mg_mqtt_connect(&state->mgr, URL, &opts, MQTT::callback_proxy, this);
    }
}

void MQTT::reconnect_proxy(void *data) {
    ((MQTT *) data)->reconnect();
}

void MQTT::main() {
    mg_timer_add(&state->mgr, 3000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, MQTT::reconnect_proxy, this);
    while (true) {
	mg_mgr_poll(&state->mgr, 1000);
    }
}
