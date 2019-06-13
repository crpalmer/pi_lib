#ifndef __SERVER_H__
#define __SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct {
    unsigned short port;
    char *(*command)(void *state, const char *cmd, struct sockaddr_in *addr, size_t addrlen);
    void *state;
} server_args_t;

#define SERVER_OK "ok"

void *
server_thread_main(void *);

#ifdef __cplusplus
};
#endif

#endif
