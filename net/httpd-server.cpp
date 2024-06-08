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

struct mg_str guess_content_type(struct mg_str path, const char *extra) {
    return ptr_to_guess_content_type(path, extra);
}

#ifdef PLATFORM_pico
const int buffer_size = 2*1024;
#else
const int buffer_size = 32*1024;
#endif

class HttpdConnection {
public:
    HttpdConnection(struct mg_connection *c, int size = buffer_size) : c(c), size(size) {
	buffer = fatal_malloc(size);
	lock = new PiMutex();
	cond = new PiCond();
	response = NULL;
	n = 0;
	enqueue_is_active = false;
    }

    ~HttpdConnection() {
	wait_for_no_enqueues();

	delete lock;
	free(buffer);
    }

    void new_response(HttpdResponse *new_response) {
	wait_for_no_enqueues();

	assert(response == NULL);
	response = new_response;
	enqueue_is_active = true;
	if (response) HttpdServer::get()->loader_enqueue(this);
    }

    int get_id() { return c->id; }

    void read() {
	assert(n == 0);
	assert(response);

	int n_read = response->read(buffer, size);

	lock->lock();
	n = n_read;
	enqueue_is_active = false;
	cond->signal();
	lock->unlock();

	HttpdServer::get()->wakeup(this);
    }

    bool is_eof() {
	return response->is_eof();
    }

    bool send_if_possible() {
	bool res = false;

	lock->lock();

	if (c->send.len >= size*2) {
	    // Don't overflow the buffer, wait for it to drain
	} else if (n > 0 && ! c->is_closing) {
            MG_DEBUG(("httpd-response-buffer: sending data"));

	    lock->unlock();
	    res = mg_send(c, buffer, n);
	    lock->lock();

	    n = 0;

	    if (! res) {
		MG_INFO(("failed to send %d bytes", n));
		c->is_resp = 0;
		response = NULL;
	    } else if (response->is_eof()) {
		c->is_resp = 0;
		delete response;
		response = NULL;
	    } else {
		assert(! enqueue_is_active);
		enqueue_is_active = true;
		HttpdServer::get()->loader_enqueue(this);
	    }
	}

	lock->unlock();

	return res;
    }

private:
    void wait_for_no_enqueues() {
	lock->lock();
	while (enqueue_is_active) cond->wait(lock);
	lock->unlock();
    }

private:
    PiMutex *lock;
    PiCond *cond;
    struct mg_connection *c;
    HttpdResponse *response;
    void *buffer;
    size_t n;
    size_t size;
    bool enqueue_is_active;
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
	    HttpdConnection *connection = waiting.front();
	    MG_DEBUG(("httpd-response-loader loading connection %d\n", connection->get_id()));
	    waiting.pop_front();
	    lock->unlock();

	    connection->read();
	    MG_DEBUG(("httpd-response-loader: completed background read"));
	}
    }

    void enqueue(HttpdConnection *connection) {
	lock->lock();
	waiting.push_back(connection);
	cond->signal();
	lock->unlock();
	MG_DEBUG(("httpd-response-loader: enqueued a new read request"));
    }

private:
    PiMutex *lock;
    PiCond *cond;
    std::list<HttpdConnection *> waiting;
};

HttpdServer::HttpdServer() {
    state = (state_t *) fatal_malloc(sizeof(*state));
    memset(state, 0, sizeof(*state));
    loader = new HttpdResponseLoader();
}

void HttpdServer::start(int port) {
    mg_log_set(MG_LL_INFO);
    mg_mgr_init(&state->mgr);
    mg_wakeup_init(&state->mgr);
    std::string listen_url = "http://0.0.0.0:" + std::to_string(port);
    if (mg_http_listen(&state->mgr, listen_url.c_str(), (mg_event_handler_t) HttpdServer::mongoose_callback_proxy, &state->mgr) == NULL) {
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
	HttpdResponse *response = get_uri(uri);

	if (! response) {
	    mg_http_reply(c, 404, "", "File not found.\n");
	} else {
	    std::string ct = response->get_content_type();
	    if (ct == "") {
		struct mg_str mg_ct = guess_content_type(hm->uri, NULL);
		ct = std::string(mg_ct.buf, mg_ct.len);
	    }
	    mg_printf(c, "%.*s %d %s\r\nContent-Type: %s\r\nContent-Length: %d\r\n%s\r\n", hm->proto.len, hm->proto.buf, response->get_status(), response->get_status_str(), ct.c_str(), response->get_n(), response->get_headers().c_str());

	    const void *raw = response->get_raw_data();
	    if (raw) {
		mg_send(c, raw, response->get_n());
	        c->is_resp = 0;
	    } else {
		if (connections[c->id] == NULL) {
		    connections[c->id] = new HttpdConnection(c);
		}
		connections[c->id]->new_response(response);
	    }
	}
	break;
    }

    case MG_EV_WRITE:
	MG_DEBUG(("callback: ev-write"));
	break;

    case MG_EV_CLOSE:
	MG_DEBUG(("callback: ev-close"));
	delete connections[c->id];
	connections.erase(c->id);
	break;

    case MG_EV_ERROR: MG_DEBUG(("callback: ev-error")); break;
    case MG_EV_OPEN: MG_DEBUG(("callback: ev-open")); break;
    case MG_EV_POLL: break;
    case MG_EV_RESOLVE: MG_DEBUG(("callback: ev-resolve")); break;
    case MG_EV_CONNECT: MG_DEBUG(("callback: ev-connect")); break;
    case MG_EV_ACCEPT: MG_DEBUG(("callback: ev-accept")); break;
    case MG_EV_TLS_HS: MG_DEBUG(("callback: ev-tls-hs")); break;
    case MG_EV_READ: MG_DEBUG(("callback: ev-read")); break;
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

    if (connections[c->id]) connections[c->id]->send_if_possible();
}

HttpdResponse *HttpdServer::get_uri(std::string fname) {
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

void HttpdServer::loader_enqueue(HttpdConnection *connection) {
    loader->enqueue(connection);
}

void HttpdServer::wakeup(HttpdConnection *connection) {
    int id = connection->get_id();

    if (! mg_wakeup(&state->mgr, id, NULL, 0)) {
	MG_INFO(("failed to wakeup connection %d", id));
    } else {
	MG_DEBUG(("triggered wakeup connection %d", id));
    }
}

HttpdResponse *HttpdDebugHandler::open(std::string path) {
    if (path == "threads") return new HttpdResponse(pi_threads_get_state());
    if (path == "free") return new HttpdResponse(std::to_string(pi_threads_get_free_ram()));
    return new HttpdResponse("Invalid request [" + path + "]");
}
