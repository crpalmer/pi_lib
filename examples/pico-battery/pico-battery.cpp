#include <stdio.h>
#include <stdlib.h>
#include "pi.h"
#include "httpd-server.h"
#include "httpd-filesystem-handler.h"
#include "net.h"
#include "pi-threads.h"
#include "pico-battery.h"
#include "wifi.h"

#include "stdin-reader.h"
#include "stdout-writer.h"
#include "threads-console.h"

class BatteryHandler : public HttpdFilenameHandler {
public:
    HttpdResponse *open() override {
	sprintf(str, "Status:  %s\nVoltage: %.2f V\n", pico_is_on_battery() ? "battery" : "powered", pico_get_vsys());
	HttpdResponse *response = new HttpdResponse(new MemoryBuffer(str));
	response->set_content_type("text/plain");
	return response;
    }

private:
    char str[256];
};

static void
threads_main(int argc, char **argv)
{
    wifi_init(CYW43_HOST_NAME);
    wifi_wait_for_connection();

    auto httpd = HttpdServer::get();
    httpd->add_file_handler("/", new HttpdRedirectHandler("/index.html"));
    httpd->add_file_handler("/index.html", new BatteryHandler());
    httpd->start();
}

int
main(int argc, char **argv)
{
    pi_init_with_threads(threads_main, argc, argv);
}
