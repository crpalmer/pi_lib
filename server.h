#ifndef __SERVER_H__
#define __SERVER_H__

typedef struct {
    unsigned short port;
    char *(*command)(void *state, const char *cmd);
    void *state;
} server_args_t;

void *
server_thread_main(void *);

#endif
