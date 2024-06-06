#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "buffer.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> cgi_params_t;

class HttpdResponse {
public:
    HttpdResponse(Buffer *buffer, bool has_headers = false) : buffer(buffer), buffer_has_headers(has_headers) {
    }
    ~HttpdResponse() { delete buffer; }

    int read(void *buf, int n) { return buffer->read(buf, n); }
    int get_n() { return buffer->get_n(); }
    bool is_eof() { return buffer->is_eof(); }
    bool has_headers() { return buffer_has_headers; }

    const void *get_raw_data() { return buffer->get_raw_data(); }

private:
    Buffer *buffer;
    bool buffer_has_headers;
};

class HttpdFileHandler {
public:
    virtual ~HttpdFileHandler() { }
    virtual HttpdResponse *open() = 0;
    virtual bool try_prefixes() { return false; }
};

class HttpdPrefixHandler {
public:
    virtual ~HttpdPrefixHandler() { }
    virtual HttpdResponse *open(std::string fname) = 0;
};

class HttpdServer {
public:
    void add_file_handler(const char *path, HttpdFileHandler *handler) {
	file_handlers[path] = handler;
    }

    void add_prefix_handler(const char *prefix, HttpdPrefixHandler *handler) {
	prefix_handlers[prefix] = handler;
    }

    void start(int port = 80);

    static HttpdServer *get();
    static void mongoose_callback_proxy(struct mg_connection *c, int ev, void *ev_data) {
	get()->mongoose_callback(c, ev, ev_data);
    }

private:
    std::map<std::string, HttpdFileHandler *> file_handlers;
    std::map<std::string, HttpdPrefixHandler *> prefix_handlers;
    struct httpd_internal_stateS *state;

private:
    HttpdServer();
    void mongoose_callback(struct mg_connection *c, int ev, void *ev_data);
    HttpdResponse *get(std::string uri);
    class HttpdResponseLoader *loader;
};

#endif
