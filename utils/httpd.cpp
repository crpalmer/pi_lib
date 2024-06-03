#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "httpd-server.h"
#include "net.h"
#include "pi-threads.h"
#include "wifi.h"

#include "time-utils.h"

class HelloFile : public HttpdFileHandler {
    HttpdResponse *open() override {
	return new HttpdResponse(new BufferBuffer("hello!\n"));
    }
};

class SecretFile : public HttpdFileHandler {
    HttpdResponse *open() override {
	return new HttpdResponse(new BufferBuffer("HTTP/1.0 200 OK\nContent-Type: text/plain\n\nThis is my secret!\n"), true);
    }
};

class TestCgiHandler : public CgiHandler {
public:
    const char *handle_request(cgi_params_t &params) {
	return get_response_filename(new BufferBuffer("cgi: Hello!"), ".txt");
    }
};

static void
threads_main(int argc, char **argv)
{
    wifi_init(CYW43_HOST_NAME);
    wifi_wait_for_connection();

    HttpdServer *httpd = HttpdServer::get();
    httpd->add_cgi_handler("/cgi-bin/test", new TestCgiHandler());
    httpd->add_file_handler("/hello.html", new HelloFile());
    httpd->add_file_handler("/secret", new SecretFile());
    httpd->start();
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
