#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mem.h"
#include "net.h"
#include "net-line-reader.h"

#include "server.h"

static void
command(void *server_as_vp, int fd, const char *line)
{
    server_args_t *server = (server_args_t *) server_as_vp;

    char *response = server->command(server->state, line);
fprintf(stderr, "response: %s\n", response);
    write(fd, response, strlen(response));
    if (! strlen(response) || response[strlen(response)] != '\n') {
	write(fd, "\n", 1);
    }
    free(response);
}

void *
server_thread_main(void *server_as_vp)
{
    server_args_t *server = (server_args_t *) server_as_vp;
    int sock;
    fd_set active_fd_set, read_fd_set;
    int i;
    struct sockaddr_in clientname;
    size_t size;
    net_line_reader_t **readers;

    sock = net_listen(server->port);
    if (sock < 0) {
	perror("net_listent");
	exit(1);
    }

    readers = fatal_malloc(sizeof(*readers) * FD_SETSIZE);

    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(sock, &active_fd_set);

    while (1) {
	/* Block until input arrives on one or more active sockets. */
	read_fd_set = active_fd_set;
	if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
	    perror("select");
	    exit(EXIT_FAILURE);
	 }

	/* Service all the sockets with input pending. */
	for (i = 0; i < FD_SETSIZE; ++i) {
	    if (FD_ISSET(i, &read_fd_set)) {
		if (i == sock) {
		    /* Connection request on original socket. */
		    int new;
		    size = sizeof(clientname);
		    new = accept(sock, (struct sockaddr *) &clientname, &size);
		    if (new < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		    }
		    fprintf(stderr,
			     "Server: connect from host %s, port %hd.\n",
			     inet_ntoa(clientname.sin_addr),
			   ntohs(clientname.sin_port));
		    FD_SET(new, &active_fd_set);
		    readers[new] = net_line_reader_new(new, command, server);
		} else {
		    /* Data arriving on an already-connected socket. */
		    if (net_line_reader_read(readers[i]) < 0){
			perror("read");
			net_line_reader_destroy(readers[i]);
			close(i);
			FD_CLR(i, &active_fd_set);
		    }
		}
	    }
	}
    }
}
