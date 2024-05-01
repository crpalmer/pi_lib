#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include "net.h"
#include "global-trace.h"

#if 0
#include <string.h>
static char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
{
    switch(sa->sa_family) {
        case AF_INET:
            inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
                    s, maxlen);
            break;

/* TODO: Use the LWIP_ define for this */
#ifndef PLATFORM_pico
        case AF_INET6:
            inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
                    s, maxlen);
            break;
#endif

        default:
            strncpy(s, "Unknown AF", maxlen);
            return NULL;
    }

    return s;
}
#endif

static int net_connect_common(const char *host, uint16_t port, int type) {
    char port_str[16];
    struct addrinfo *addr_info;
    struct addrinfo hints = {
        .ai_flags = 0,
        .ai_family = AF_UNSPEC,
        .ai_socktype = type,
        .ai_protocol = 0
    };

    sprintf(port_str, "%u", port);
    if (getaddrinfo(host, port_str, &hints, &addr_info) != 0) return -1;

    for (struct addrinfo *a = addr_info; a != NULL; a = a->ai_next) {
	int fd;

	if ((fd = socket(a->ai_family, a->ai_socktype, a->ai_protocol)) >= 0) {
	    struct timeval to = { .tv_sec = 30 };
    	    if (type == SOCK_DGRAM && (
                 setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof(to)) < 0 ||
	         setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to)) < 0
               ))
	    {
		perror("setsockopt");
	    } else if (connect(fd, a->ai_addr, a->ai_addrlen) < 0) {
		perror("connect");
	    } else {
		freeaddrinfo(addr_info);
		return fd;
	    }
	    closesocket(fd);
         }
     }

     freeaddrinfo(addr_info);
     return -1;
}

int
net_connect_tcp(const char *host, uint16_t port)
{
    return net_connect_common(host, port, SOCK_STREAM);
}

int
net_connect_udp(const char *host, uint16_t port)
{
    return net_connect_common(host, port, SOCK_DGRAM);
}

int
net_listen(uint16_t port)
{
    int sock;
    struct sockaddr_in name;
    int optval;

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
	perror("socket");
	exit(1);
    }

    optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

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
