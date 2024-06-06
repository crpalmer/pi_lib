#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "buffer.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> cgi_params_t;

class HttpdConnection;

class HttpdResponse {
public:
    HttpdResponse(Buffer *buffer) : buffer(buffer) {
    }
    ~HttpdResponse() { delete buffer; }

    int read(void *buf, int n) { return buffer->read(buf, n); }
    int get_n() { return buffer->get_n(); }
    bool is_eof() { return buffer->is_eof(); }

    const void *get_raw_data() { return buffer->get_raw_data(); }

private:
    Buffer *buffer;
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

protected:
    void loader_enqueue(HttpdConnection *connection);
    void wakeup(HttpdConnection *connection);

    friend class HttpdConnection;

private:
    std::map<std::string, HttpdFileHandler *> file_handlers;
    std::map<std::string, HttpdPrefixHandler *> prefix_handlers;
    std::map<int, HttpdConnection *> connections;
    struct httpd_internal_stateS *state;
    class HttpdResponseLoader *loader;

private:
    HttpdServer();
    void mongoose_callback(struct mg_connection *c, int ev, void *ev_data);
    HttpdResponse *get(std::string uri);
};

#endif
