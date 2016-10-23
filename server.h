#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h>
#include <sys/socket.h>

typedef struct {
    unsigned short port;
    char *(*command)(void *state, const char *cmd);
    void *state;
    void *(*on_connect)(void *state, struct sockaddr_in *addr, size_t addrlen);
} server_args_t;

#define SERVER_OK "ok"

void *
server_thread_main(void *);

#endif
