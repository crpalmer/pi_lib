#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mem.h"
#include "net.h"
#include "net-line-reader.h"

#include "server.h"

typedef struct {
    int fd;
    server_args_t *server;
    struct sockaddr_in *addr;
    size_t size;
} connection_t;

static bool
command(void *c_as_vp, int fd, const char *line)
{
    connection_t *c = (connection_t *) c_as_vp;
    server_args_t *server = c->server;
    char *response = server->command(server->state, line, c->addr, c->size);
    bool success;

printf("response: %s\n", response);

    if (! strlen(response) || response[strlen(response)] != '\n') {
	char *response2 = malloc(strlen(response) + 2);
	sprintf(response2, "%s\n", response);
	free(response);
	response = response2;
    }

    success = write(fd, response, strlen(response)) == strlen(response);
    free(response);

    return success;
}

void *
connection_main(void *c_as_vp)
{
    connection_t *c = (connection_t *) c_as_vp;
    int fd = c->fd;
    net_line_reader_t *reader;

    reader = net_line_reader_new(fd, command, c_as_vp);
    while (net_line_reader_read(reader) >= 0) {
    }
    net_line_reader_destroy(reader);
    close(fd);
    free(c);

    return NULL;
}

void *
server_thread_main(void *server_as_vp)
{
    server_args_t *server = (server_args_t *) server_as_vp;
    int sock;

    signal(SIGPIPE, SIG_IGN);

    sock = net_listen(server->port);
    if (sock < 0) {
	perror("net_listent");
	exit(1);
    }

    while (1) {
	connection_t *c;
	pthread_t thread;
	int fd;
	struct sockaddr_in clientname;
	size_t size = sizeof(clientname);

	fd = accept(sock, (struct sockaddr *) &clientname, &size);
	if (fd < 0) {
	    perror("accept");
	    exit(EXIT_FAILURE);
	}

	fprintf(stderr,
		 "Server: connect from host %s, port %hu.\n",
		 inet_ntoa(clientname.sin_addr),
	       ntohs(clientname.sin_port));

	c = malloc(sizeof(*c));
	c->fd = fd;
	c->server = server;
	c->addr = (struct sockaddr_in *) fatal_malloc(size);
	memcpy(c->addr, &clientname, size);
	c->size = size;

	pthread_create(&thread, NULL, connection_main, c);
	pthread_detach(thread);
    }
}
