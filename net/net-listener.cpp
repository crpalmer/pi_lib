#include "net.h"
#include "consoles.h"
#include "time-utils.h"
#include "wifi.h"

#include "net-listener.h"

void NetListener::main() {
    consoles_write_str("net: Starting.\n");

    while (1) {
        uint16_t port = 4567;

#ifdef PLATFORM_pico
	while (! wifi_is_connected()) {
	    consoles_printf("net: Waiting for WiFi to become available.\n");
	    wifi_wait_for_connection();
	}
#endif

        consoles_printf("net: Listening on port %u\n", port);
        int l_fd = net_listen(port);

        while (1) {
            int fd;
            struct sockaddr_in client;
            socklen_t size = sizeof(client);

            consoles_write_str("net: Waiting for connection.\n");
            if ((fd = accept(l_fd, (struct sockaddr *) &client, &size)) < 0) {
                perror("net_accept");
                ms_sleep(5*1000);
            } else {
                consoles_printf("connect from host %s, port %hu.\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		accepted(fd);
            }
        }
    }
}
