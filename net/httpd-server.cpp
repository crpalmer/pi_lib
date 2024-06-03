#include <memory.h>
#include "pi.h"
#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "httpd-server.h"
#include "mem.h"
#include "pi-threads.h"
#include "string-utils.h"

typedef struct httpd_internal_stateS {
    tCGI *cgi;
} state_t;

static PiMutex *lock;
static HttpdServer *instance;

HttpdServer::HttpdServer() {
    state = (state_t *) fatal_malloc(sizeof(*state));
    memset(state, 0, sizeof(*state));
}

extern "C" int fs_open_custom(fs_file *file, const char *name) {
    Buffer *b = instance->claim_response_buffer(name);
    if (b) {
	file->flags |= FS_FILE_FLAGS_CUSTOM;
	file->pextension = b;
	file->len = b->get_n();
	return true;
    }
    return false;
}

extern "C" int fs_read_custom(fs_file *file, char *buffer, int count) {
    if (((Buffer *) file->pextension)->read(buffer, count)) {
	return count;
    } else {
	return FS_READ_EOF;
    }
}

extern "C" void fs_close_custom(fs_file *file) {
    if (file->pextension) delete ((Buffer *) file->pextension);
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

const char *HttpdServer::cgi_handler_proxy(int handler_index, int n_params, char *names[], char *values[]) {
    cgi_params_t params;

    for (int i = 0; i < n_params; i++) params[names[i]] = values[i];
    return instance->cgi_handler(handler_index, params);
}

const char *HttpdServer::cgi_handler(int handler_index, cgi_params_t &params) {
    CgiHandler *cgi_handler = cgi_handlers[state->cgi[handler_index].pcCGIName];
    return cgi_handler->handle_request(params);
}

void HttpdServer::start(int port) {
    httpd_init();

    if (cgi_handlers.size() > 0) {
	state->cgi = (tCGI *) fatal_malloc(sizeof(*state->cgi) * cgi_handlers.size());
	int i = 0;
	for (std::pair<std::string, CgiHandler *> p : cgi_handlers) {
	    state->cgi[i].pcCGIName = fatal_strdup(p.first.c_str());
	    state->cgi[i].pfnCGIHandler = cgi_handler_proxy;
	    i++;
	}
	http_set_cgi_handlers(state->cgi, cgi_handlers.size());
    }
}

void HttpdServer::store_response_buffer(CgiHandler *cgi_handler, Buffer *buffer) {
    if (cgi_active_buffers[cgi_handler]) delete cgi_active_buffers[cgi_handler];
    cgi_active_buffers[cgi_handler] = buffer;
}

const char *CgiHandler::get_response_filename(Buffer *b, const char *extension) {
    HttpdServer::get()->store_response_buffer(this, b);

    if (lwip_filename) free(lwip_filename);
    lwip_filename = maprintf("%p%s%s", this, extension && extension[0] != '.' ? "." : "", extension ? extension : "");
    return lwip_filename;
}

Buffer *HttpdServer::claim_response_buffer(std::string fname) {
    // Check for any active cgi responses that are pending

    if (fname[0] != '/') {
	CgiHandler *cgi_handler = (CgiHandler *) stoul(fname, 0, 16);
	if (cgi_active_buffers[cgi_handler]) {
	    Buffer *b = cgi_active_buffers[cgi_handler];
	    cgi_active_buffers[cgi_handler] = NULL;
	    return b;
	}
    }

    // Check for any registered full filenmae

    if (file_handlers[fname]) {
	return file_handlers[fname]->open();
    }

    // Check for any hierarchy handlers
    // TODO

    return NULL;
}

