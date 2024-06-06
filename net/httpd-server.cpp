#include <memory.h>
#include <list>
#include "pi.h"
#define NOT_IN_MONGOOSE_C
#include "mongoose.h"
#include "consoles.h"
#include "mem.h"
#include "pi-threads.h"
#include "string-utils.h"
#include "httpd-server.h"

typedef struct httpd_internal_stateS {
    struct mg_mgr mgr;
} state_t;

static PiMutex *lock;
static HttpdServer *instance;

struct mg_str guess_content_type(struct mg_str path, const char *extra) {
    return ptr_to_guess_content_type(path, extra);
}

#ifdef PLATFORM_pico
const int buffer_size = 1*1024;
#else
const int buffer_size = 10*1024;
#endif

class HttpdResponseBuffer {
public:
    HttpdResponseBuffer(struct mg_mgr *mgr, struct mg_connection *c, HttpdResponse *response, int size = buffer_size) : mgr(mgr), c(c), response(response), size(size) {
	buffer = fatal_malloc(size);
	n  = 0;
    }

    ~HttpdResponseBuffer() {
	free(buffer);
    }

    void read() {
	n = response->read(buffer, size);
    }

    bool is_eof() {
	return response->is_eof();
    }

    void wakeup() {
	if (! mg_wakeup(mgr, c->id, this, sizeof(this))) {
	    MG_INFO(("httpd-response-buffer: failed to wakeup"));
        } else {
	    MG_DEBUG(("httpd-response-buffer: triggered wakeup"));
	}
    }

    bool connection_send() {
        MG_DEBUG(("httpd-resppnse-buffer: sending data"));
	if (c->is_closing) return false;

	bool res = mg_send(c, buffer, n);
	if (! res || response->is_eof()) c->is_resp = 0;
	return res;
    }

private:
    struct mg_mgr *mgr;
    struct mg_connection *c;
    HttpdResponse *response;
    void *buffer;
    int n;
    int size;
};

class HttpdResponseLoader : public PiThread {
public:
    HttpdResponseLoader() : PiThread("httpd-loader") {
	lock = new PiMutex();
	cond = new PiCond();
	start();
    }

    void main(void) override {
	while (true) {
	    lock->lock();
	    while (waiting.size() == 0) cond->wait(lock);
	    HttpdResponseBuffer *hrb = waiting.front();
	    waiting.pop_front();
	    lock->unlock();

	    hrb->read();

	    MG_DEBUG(("httpd-response-loader: completed background read"));
	    lock->lock();
	    ready.push_back(hrb);
	    hrb->wakeup();
	    lock->unlock();
	}
    }

    HttpdResponseBuffer *get_ready_response() {
    	lock->lock();
	HttpdResponseBuffer *hrb = ready.front();
	if (hrb) ready.pop_front();
	lock->unlock();

	if (hrb) MG_DEBUG(("httpd-response-buffer: giving up a got ready response"));
	return hrb;
    }

    void enqueue(HttpdResponseBuffer *hrb) {
	lock->lock();
	waiting.push_back(hrb);
	cond->signal();
	lock->unlock();
	MG_DEBUG(("httpd-response-loader: enqueued a new read request"));
    }

private:
    PiMutex *lock;
    PiCond *cond;
    std::list<HttpdResponseBuffer *> ready;
    std::list<HttpdResponseBuffer *> waiting;
};

HttpdServer::HttpdServer() {
    state = (state_t *) fatal_malloc(sizeof(*state));
    memset(state, 0, sizeof(*state));
    loader = new HttpdResponseLoader();
}

/* TODO
 *
 * Add a global pi_threads_singleton lock that is always initialized and then this can be
 * if (! lock)
 *     lock singleton lock
 *     double check lock
 * ...
 * and avoid what I think are some race conditions in there.
 *
 * TODO
 *
 * Go one further and add a template Singleton generator that does all this for you?
 */

HttpdServer *HttpdServer::get() {
    if (! lock) {
        PiMutex *new_lock = new PiMutex();
	lock = new_lock;
	lock->lock();
	if (lock != new_lock) {
	    // Someone else beat me to creating the lock, try again
	    delete new_lock;
	}
    } else {
	lock->lock();
    }

    if (! instance) {
        instance = new HttpdServer();
    }

    lock->unlock();
    return instance;
}

void HttpdServer::start(int port) {
    mg_log_set(MG_LL_DEBUG);
    mg_mgr_init(&state->mgr);
    mg_wakeup_init(&state->mgr);
    if (mg_http_listen(&state->mgr, "http://0.0.0.0:9999", (mg_event_handler_t) HttpdServer::mongoose_callback_proxy, &state->mgr) == NULL) {
	consoles_fatal_printf("Couldn't start the web server.\n");
    }
    while (true) mg_mgr_poll(&state->mgr, 10000);
}

void HttpdServer::mongoose_callback(struct mg_connection *c, int ev, void *ev_data) {
    switch (ev) {
    case MG_EV_HTTP_MSG: {
	MG_DEBUG(("callback: ev-msg"));
	struct mg_http_message *hm = (struct mg_http_message *) ev_data;
	std::string uri = std::string(hm->uri.buf, hm->uri.len);
	HttpdResponse *response = get(uri);

	if (! response) {
	    mg_http_reply(c, 404, "", "File not found.\n");
	} else {
	    struct mg_str ct = guess_content_type(hm->uri, NULL);

	    mg_printf(c, "%.*s 200 OK\r\nContent-Type: %.*s\r\nContent-Length: %d\r\n\r\n", hm->proto.len, hm->proto.buf, ct.len, ct.buf, response->get_n());
	    const void *raw = response->get_raw_data();
	    if (raw) {
		mg_send(c, raw, response->get_n());
	        c->is_resp = 0;
	    } else {
		loader->enqueue(new HttpdResponseBuffer(&state->mgr, c, response));
	    }
	}
	break;
    }

    case MG_EV_ERROR: MG_DEBUG(("callback: ev-error")); break;
    case MG_EV_OPEN: MG_DEBUG(("callback: ev-open")); break;
    case MG_EV_POLL: break;
    case MG_EV_RESOLVE: MG_DEBUG(("callback: ev-resolve")); break;
    case MG_EV_CONNECT: MG_DEBUG(("callback: ev-connect")); break;
    case MG_EV_ACCEPT: MG_DEBUG(("callback: ev-accept")); break;
    case MG_EV_TLS_HS: MG_DEBUG(("callback: ev-tls-hs")); break;
    case MG_EV_READ: MG_DEBUG(("callback: ev-read")); break;
    case MG_EV_WRITE: MG_DEBUG(("callback: ev-write")); break;
    case MG_EV_CLOSE: MG_DEBUG(("callback: ev-close")); break;
    case MG_EV_HTTP_HDRS: MG_DEBUG(("callback: ev-http-hdrs")); break;
    case MG_EV_WS_OPEN: MG_DEBUG(("callback: ev-ws-open")); break;
    case MG_EV_WS_MSG: MG_DEBUG(("callback: ev-ws-msg")); break;
    case MG_EV_WS_CTL: MG_DEBUG(("callback: ev-ws-ctl")); break;
    case MG_EV_MQTT_CMD: MG_DEBUG(("callback: ev-mqtt-cmd")); break;
    case MG_EV_MQTT_MSG: MG_DEBUG(("callback: ev-mqtt-msg")); break;
    case MG_EV_MQTT_OPEN: MG_DEBUG(("callback: ev-mqtt-open")); break;
    case MG_EV_SNTP_TIME: MG_DEBUG(("callback: ev-sntp-time")); break;
    case MG_EV_WAKEUP: MG_DEBUG(("callback: ev-wakeup")); break;
    }

    HttpdResponseBuffer *rb;
    while ((rb = loader->get_ready_response()) != NULL) {
	if (rb->connection_send() && ! rb->is_eof()) {
	    loader->enqueue(rb);
	} else {
	    delete rb;
	}
    }
}

HttpdResponse *HttpdServer::get(std::string fname) {
    // Check for any registered full filenmae

    if (file_handlers[fname]) {
	return file_handlers[fname]->open();
    }

    // Check for any prefix handlers
    std::string prefix = fname;
    while (true) {
	size_t last_slash = prefix.find_last_of("/");

	if (last_slash == std::string::npos) {
	    break;
	}

	prefix = prefix.substr(0, last_slash);

	if (prefix_handlers[prefix]) {
	    std::string suffix = fname.substr(prefix.length()+1);
	    HttpdResponse *response = prefix_handlers[prefix]->open(suffix);
	    if (response) return response;
	}
    }

    return NULL;
}
