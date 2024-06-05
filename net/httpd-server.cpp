#include <memory.h>
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

HttpdServer::HttpdServer() {
    state = (state_t *) fatal_malloc(sizeof(*state));
    memset(state, 0, sizeof(*state));
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
    if (mg_http_listen(&state->mgr, "http://0.0.0.0:9999", (mg_event_handler_t) HttpdServer::mongoose_callback_proxy, &state->mgr) == NULL) {
	consoles_fatal_printf("Couldn't start the web server.\n");
    }
    while (true) mg_mgr_poll(&state->mgr, 1000);
}

void HttpdServer::mongoose_callback(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
	struct mg_http_message *hm = (struct mg_http_message *) ev_data;
	std::string uri = std::string(hm->uri.buf, hm->uri.len);
	HttpdResponse *response = get(uri);

	printf("Got request: %s %p\n", uri.c_str(), response);
	if (! response) {
	    mg_http_reply(c, 404, "", "File not found.\n");
	} else {
	    struct mg_str ct = guess_content_type(hm->uri, NULL);

	    mg_printf(c, "%.*s 200 OK\r\nContent-Type: %.*s\r\nContent-Length: %d\r\n\r\n", hm->proto.len, hm->proto.buf, ct.len, ct.buf, response->get_n());
	    while (! response->is_eof()) {
		static char *buffer[1024];
		int n = response->read(buffer, sizeof(buffer));
		mg_send(c, buffer, n);
	    }
	    c->is_resp = 0;
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
