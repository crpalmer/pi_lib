#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "buffer.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> cgi_params_t;

class HttpdConnection;

class HttpdResponse {
public:
    HttpdResponse(Buffer *buffer) : buffer(buffer) { }
    HttpdResponse(std::string string) : HttpdResponse(new StrdupBuffer(string)) { }

    ~HttpdResponse() { delete buffer; }

    void set_status(int status) {
	this->status = status;
    }

    void add_header(std::string header) {
	headers += header;
	headers += "\r\n";
    }

    void set_content_type(std::string content_type) {
	this->content_type = content_type;
    }

    int read(void *buf, int n) { return buffer->read(buf, n); }
    int get_n() { return buffer->get_n(); }
    bool is_eof() { return buffer->is_eof(); }

    const void *get_raw_data() { return buffer->get_raw_data(); }

    auto get_status() { return status; }
    auto get_content_type() { return content_type; }
    auto get_headers() { return headers; }

    const char *get_status_str() {
	switch(status) {
	case 302: return "FOUND";
	case 404: return "Not Found";
	case 500: return "Internal Server Error";
	default: return "OK";
	}
    }

private:
    Buffer *buffer;
    int status = 200;
    std::string content_type;
    std::string headers;
};

class HttpdFilenameHandler {
public:
    virtual ~HttpdFilenameHandler() { }
    virtual HttpdResponse *open() = 0;
};

class HttpdFileHandler : public HttpdFilenameHandler {
public:
    HttpdFileHandler(std::string filename) : filename(filename) { }

    virtual HttpdResponse *open() {
	FileBuffer *buffer = file_buffer_open(filename);
	if (buffer) return new HttpdResponse(buffer);
	else return NULL;
    }

private:
    std::string filename;
};

class HttpdRedirectHandler : public HttpdFilenameHandler {
public:
    HttpdRedirectHandler(std::string destination) : destination(destination) { }

    virtual HttpdResponse *open() {
	HttpdResponse *response = new HttpdResponse("");
	response->set_status(302);
	response->add_header("Location: " + destination);
	return response;
    }

private:
    std::string destination;
};

class HttpdPrefixHandler {
public:
    virtual ~HttpdPrefixHandler() { }
    virtual HttpdResponse *open(std::string path) = 0;
};

class HttpdDebugHandler : public HttpdPrefixHandler {
public:
    HttpdResponse *open(std::string path) override;
};

class HttpdServer {
public:
    static HttpdServer *get() {
	static HttpdServer instance;
	return &instance;
    }

    void add_file_handler(std::string path, HttpdFilenameHandler *handler) {
	file_handlers[path] = handler;
    }

    void add_prefix_handler(std::string prefix, HttpdPrefixHandler *handler) {
	prefix_handlers[prefix] = handler;
    }

    void start(int port = 80);

    static void mongoose_callback_proxy(struct mg_connection *c, int ev, void *ev_data) {
	get()->mongoose_callback(c, ev, ev_data);
    }

protected:
    void loader_enqueue(HttpdConnection *connection);
    void wakeup(HttpdConnection *connection);

    friend class HttpdConnection;

private:
    std::map<std::string, HttpdFilenameHandler *> file_handlers;
    std::map<std::string, HttpdPrefixHandler *> prefix_handlers;
    std::map<int, HttpdConnection *> connections;
    struct httpd_internal_stateS *state;
    class HttpdResponseLoader *loader;

private:
    HttpdServer();
    void mongoose_callback(struct mg_connection *c, int ev, void *ev_data);
    HttpdResponse *get_uri(std::string uri);
};

#endif
