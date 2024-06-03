#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "buffer.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> cgi_params_t;

extern "C" int fs_open_custom(struct fs_file *file, const char *name);

class CgiHandler {
public:
    virtual ~CgiHandler() { }
    virtual const char *handle_request(cgi_params_t &params) = 0;

    const char *get_response_filename(Buffer *b, const char *extension = NULL);

private:
    char *lwip_filename;
};

class HttpdFileHandler {
public:
    virtual ~HttpdFileHandler() { }
    virtual Buffer *open() = 0;
    virtual bool try_prefixes() { return false; }
};

class HttpdServer {
public:
    void add_cgi_handler(const char *path, CgiHandler *handler) {
	cgi_handlers[path] = handler;
    }

    void add_file_handler(const char *path, HttpdFileHandler *handler) {
	file_handlers[path] = handler;
    }

    void start(int port = 80);

    static HttpdServer *get();

private:
    std::map<std::string, CgiHandler *> cgi_handlers;
    std::map<CgiHandler *, Buffer *> cgi_active_buffers;
    std::map<std::string, HttpdFileHandler *> file_handlers;
    struct httpd_internal_stateS *state;

private:
    HttpdServer();

    static const char *cgi_handler_proxy(int handler_index, int n_params, char *names[], char *values[]);

protected:
    const char *cgi_handler(int handler_index, cgi_params_t &params);
    void store_response_buffer(CgiHandler *handler, Buffer *buffeR);
    Buffer *claim_response_buffer(std::string name); 

    friend const char *CgiHandler::get_response_filename(Buffer *b, const char *extension);

    friend int fs_open_custom(struct fs_file *file, const char *name);
    friend const char *cgi_handler_proxy(int handler_index, int n_params, char *names[], char *values[]);
};

#endif
