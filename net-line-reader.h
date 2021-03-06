#ifndef __NET_LINE_READER_H__
#define __NET_LINE_READER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct net_line_readerS net_line_reader_t;

typedef bool (*net_line_reader_callback_t)(void *data, int socket, const char *line);

net_line_reader_t *
net_line_reader_new(int fd, net_line_reader_callback_t cb, void *data);

int
net_line_reader_read(net_line_reader_t *r);

void
net_line_reader_destroy(net_line_reader_t *r);

#ifdef __cplusplus
};
#endif

#endif
