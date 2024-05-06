#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "consoles.h"
#include "mem.h"
#include "net.h"
#include "net-line-reader.h"
#include "pi-threads.h"

#include "server.h"

typedef struct {
    int fd;
    server_args_t *server;
    struct sockaddr_in *addr;
    size_t size;
} connection_t;

void
connection_main(void *c_as_vp)
{
    connection_t *c = (connection_t *) c_as_vp;
    int fd = c->fd;
    net_line_reader_t *reader;
    const char *cmd;

    reader = net_line_reader_new(fd);
    while ((cmd = net_line_reader_read(reader)) != NULL) {
	server_args_t *server = c->server;
	char *response = server->command(server->state, cmd, c->addr, c->size);

	if (! strlen(response) || response[strlen(response)] != '\n') {
	    char *response2 = fatal_malloc(strlen(response) + 2);
	    sprintf(response2, "%s\n", response);
	    fatal_free(response);
	    response = response2;
	}

	if (send(fd, response, strlen(response), 0) != strlen(response)) {
	    consoles_printf("Write failed.\n");
	}
	fatal_free(response);
    }
    net_line_reader_destroy(reader);
    closesocket(fd);
    fatal_free(c);
}

void
server_thread_main(void *server_as_vp)
{
    server_args_t *server = (server_args_t *) server_as_vp;
    int sock;

    signal(SIGPIPE, SIG_IGN);

    sock = net_listen(server->port);
    if (sock < 0) {
	perror("net_listen");
	exit(1);
    }

    while (1) {
	connection_t *c;
	int fd;
	struct sockaddr_in clientname;
	socklen_t size = sizeof(clientname);

	fd = accept(sock, (struct sockaddr *) &clientname, &size);
	if (fd < 0) {
	    perror("accept");
	    exit(EXIT_FAILURE);
	}

	fprintf(stderr,
		 "Server: connect from host %s, port %hu.\n",
		 inet_ntoa(clientname.sin_addr),
	       ntohs(clientname.sin_port));

	c = fatal_malloc(sizeof(*c));
	c->fd = fd;
	c->server = server;
	c->addr = (struct sockaddr_in *) fatal_malloc(size);
	memcpy(c->addr, &clientname, size);
	c->size = size;

	pi_thread_create("connection", connection_main, c);
    }
}
