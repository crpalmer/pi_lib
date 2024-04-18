#ifndef __NET_LINE_READER_H__
#define __NET_LINE_READER_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct net_line_readerS net_line_reader_t;

net_line_reader_t *net_line_reader_new(int fd);
const char *net_line_reader_read(net_line_reader_t *r);
void net_line_reader_destroy(net_line_reader_t *);

#ifdef __cplusplus
};
#endif

#endif
