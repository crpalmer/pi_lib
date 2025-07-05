#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

/** @defgroup HTTP HTTP Server
 *  @ingroup Net
 *
 * @{
 */

#include "buffer.h"

#include <map>
#include <string>

typedef std::map<std::string, std::string> cgi_params_t;

class HttpdConnection;

/** The response to a HTTP request.
 *
 * This class is used to generate a HTTP response.  Normally you would provide
 * an instance of a Buffer class to stream the data but there is also a helper
 * constructor that will let you pass a fixed string.
 *
 * If returning HTML, the caller must take care to ensure all HTML is properly
 * escaped so as to not have any potential vulnerabilities.
 */

class HttpdResponse {
public:
    /** A response that will provide a fixed buffer.
     *
     * @param buffer - The buffer object that contains the response data
     */

    HttpdResponse(Buffer *buffer) : buffer(buffer) { }

    /** A response that will provide a fixed string.
     *
     * @param string - The string that contains the response data
     */

    HttpdResponse(std::string string) : HttpdResponse(new StrdupBuffer(string)) { }

    ~HttpdResponse() { delete buffer; }

    /** Change the HTTP response status
     * 
     * @param status - Set the HTTP protocol response status (number, 200 by default)
     */

    void set_status(int status) {
	this->status = status;
    }

    /** Add a line to the HTTP header response.
     *
     * The header is normally \c Key: \c Value.  It should may not contain a newline.
     *
     * @param header - the header line to add.
     */

    void add_header(std::string header) {
	headers += header;
	headers += "\r\n";
    }

    /** Helper to add a \c Content-Type: header line.
     *
     * @param content_type - The value of the content-type header
     */

    void set_content_type(std::string content_type) {
	this->content_type = content_type;
    }

    /// @cond INTERNAL

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

    /// @endcond

private:
    Buffer *buffer;
    int status = 200;
    std::string content_type;
    std::string headers;
};

/** An abstract class that provides a response to a filename mapping.
 */

class HttpdFilenameHandler {
public:
    virtual ~HttpdFilenameHandler() { }

    /** Create the response object
     *
     * @returns the response to the HTTP request
     */

    virtual HttpdResponse *open() = 0;
};

/** A \c HttpdFilenameHandler that serves a local file.
 */

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

/** A \c HttpdFilenameHandler that generate a redirect.
 */

class HttpdRedirectHandler : public HttpdFilenameHandler {
public:
    /** @param destination - The URL to which to redirect.
     */

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

/** An abstract class that provides a response to a prefix mapping.
 */

class HttpdDebugHandler : public HttpdPrefixHandler {
public:
    HttpdResponse *open(std::string path) override;
};

/** A singleton that will start and operate a HTTP server.
 *
 * This object has negligle cost until you start it but after
 * starting it will provide a full implementation of the HTTP
 * server.
 *
 * The caller will serve responses by creating handlers that
 * either respond to a single fixed path or handlers that
 * handle whole hierarchy prefixes.
 */

class HttpdServer {
public:
    /** Get the \c HttpdServer singleton
     */

    static HttpdServer *get() {
	static HttpdServer instance;
	return &instance;
    }

    /** Add a handler for a specific path.
     *
     * Add the \c handler which will be called if a request is made for the
     * path \c path.
     *
     * If the same path is called for multiple handlers, only the last
     * one registered will be called.
     *
     * @param path - The path to declare a handler for
     * @param handler - The handler that will be invoked
     */

    void add_file_handler(std::string path, HttpdFilenameHandler *handler) {
	file_handlers[path] = handler;
    }

    /** Add a handler for a specific prefix.
     *
     * Add the \c handler to handle requests for the requested path \c prefix.
     *
     * A \c prefix is a path prefix of the request page.  For example, the
     * request for \b /aaa/bb/c will search for prefix handlers \b /aaa/bb
     * and then \b /aaa and finally \b /.
     *
     * If the same prefix is called for multiple handlers, only the last
     * one registered will be called.
     *
     * @param prefix - The path prefix to add a handler for
     * @param handler - The handler that will be invoked
     */

    void add_prefix_handler(std::string prefix, HttpdPrefixHandler *handler) {
	prefix_handlers[prefix] = handler;
    }

    /** Actually start the web server running.
     *
     * @param port - The port on which to listen for HTTP connections
     */

    void start(int port = 80);

    /// @cond INTERNAL

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

    /// @endcond
};

/** @} */

#endif
