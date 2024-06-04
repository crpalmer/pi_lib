#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "buffer.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> cgi_params_t;

extern "C" int fs_open_custom(struct fs_file *file, const char *name);

class HttpdResponse {
public:
    HttpdResponse(Buffer *buffer, bool has_headers = false) : buffer(buffer), buffer_has_headers(has_headers) {
    }
    ~HttpdResponse() { delete buffer; }

    int read(void *buf, int n) { return buffer->read(buf, n); }
    int get_n() { return buffer->get_n(); }
    bool has_headers() { return buffer_has_headers; }

private:
    Buffer *buffer;
    bool buffer_has_headers;
};

class CgiHandler {
public:
    virtual ~CgiHandler() { }
    virtual const char *handle_request(cgi_params_t &params) = 0;

    const char *get_response_filename(HttpdResponse *response, const char *extension = NULL);
    const char *get_response_filename(Buffer *buffer, const char *extension = NULL);
    const char *get_response_filename(const char *text, const char *extension = ".txt");

private:
    char *lwip_filename;
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
    void add_cgi_handler(const char *path, CgiHandler *handler) {
	cgi_handlers[path] = handler;
    }

    void add_file_handler(const char *path, HttpdFileHandler *handler) {
	file_handlers[path] = handler;
    }

    void add_prefix_handler(const char *prefix, HttpdPrefixHandler *handler) {
	prefix_handlers[prefix] = handler;
    }

    void start(int port = 80);

    static HttpdServer *get();

private:
    std::map<std::string, CgiHandler *> cgi_handlers;
    std::map<CgiHandler *, HttpdResponse *> cgi_active_responses;
    std::map<std::string, HttpdFileHandler *> file_handlers;
    std::map<std::string, HttpdPrefixHandler *> prefix_handlers;
    struct httpd_internal_stateS *state;

private:
    HttpdServer();

    static const char *cgi_handler_proxy(int handler_index, int n_params, char *names[], char *values[]);

protected:
    const char *cgi_handler(int handler_index, cgi_params_t &params);
    void store_response(CgiHandler *handler, HttpdResponse *response);
    HttpdResponse *claim_response(std::string name); 

    friend const char *CgiHandler::get_response_filename(HttpdResponse *response, const char *extension);

    friend int fs_open_custom(struct fs_file *file, const char *name);
    friend const char *cgi_handler_proxy(int handler_index, int n_params, char *names[], char *values[]);
};

#endif
