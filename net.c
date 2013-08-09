#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "net.h"
#include "global-trace.h"

int
net_listen(uint16_t port)
{
    int sock;
    struct sockaddr_in name;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
	perror("socket");
	exit(1);
    }

    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &name, sizeof(name)) < 0) {
	perror("bind");
	exit(1);
    }

    if (listen(sock, 1) < 0) {
	perror("listen");
	exit(1);
    }

    if (global_trace) fprintf(stderr, "%s: on socket %d port %d\n", __func__, sock, port);

    return sock;
}
