#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "httpd-server.h"
#include "httpd-filesystem-handler.h"
#include "net.h"
#include "pi-threads.h"
#include "wifi.h"

#include "stdin-reader.h"
#include "stdout-writer.h"
#include "threads-console.h"

class HelloFile : public HttpdFileHandler {
public:
    HttpdResponse *open() override {
	return new HttpdResponse(new BufferBuffer("hello!\n"));
    }
};

class FilesystemFileHandler : public HttpdFileHandler {
public:
    FilesystemFileHandler(std::string path) : path(path) { }

    HttpdResponse *open() override {
	Buffer *b = buffer_file_open(path);
	if (! b) {
	    fprintf(stderr, "Failed to open: %s\n", path.c_str());
	    return NULL;
	}

printf("returning %s: %d\n", path.c_str(), b->get_n());
	return new HttpdResponse(b);
    }

private:
    std::string path;
};

class SecretFile : public HttpdFileHandler {
public:
    HttpdResponse *open() override {
	return new HttpdResponse(new BufferBuffer("HTTP/1.0 200 OK\nContent-Type: text/plain\n\nThis is my secret!\n"), true);
    }
};

class MyConsole : public PiThread, public ThreadsConsole {
public:
    MyConsole() : PiThread("console"), ThreadsConsole(new StdinReader(), new StdoutWriter()) { start(); }
    void main(void) { ThreadsConsole::main(); }
};

static void
threads_main(int argc, char **argv)
{
    new MyConsole();

    wifi_init(CYW43_HOST_NAME);
    wifi_wait_for_connection();

    HttpdServer *httpd = HttpdServer::get();
    httpd->add_file_handler("/hello.html", new HelloFile());
    httpd->add_file_handler("/secret", new SecretFile());
    httpd->add_file_handler("/audio/laugh.wav", new FilesystemFileHandler("/laugh.wav"));
    httpd->add_prefix_handler("/www", new HttpdFilesystemHandler("/www"));
    httpd->add_prefix_handler("/tmp/wave", new HttpdFilesystemHandler("/tmp/2121_wave_cafe"));
    httpd->add_prefix_handler("/www/wave", new HttpdFilesystemHandler("/2121_wave_cafe"));
    httpd->start(9999);
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}