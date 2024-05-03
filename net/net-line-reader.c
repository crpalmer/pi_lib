#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include "mem.h"
#include "net.h"
#include "net-line-reader.h"

#define MIN_READ_SIZE	1024

struct net_line_readerS {
    int   fd;
    char *buf;
    int   a_buf, n_buf;
};

net_line_reader_t *
net_line_reader_new(int fd)
{
    net_line_reader_t *r = fatal_malloc(sizeof(*r));

    r->fd = fd;
    r->a_buf = 1024;
    r->n_buf = 0;
    r->buf = malloc(r->a_buf);

    return r;
}

const char *
net_line_reader_read(net_line_reader_t *r)
{
    int n_bytes;

    if (r->a_buf - r->n_buf < MIN_READ_SIZE) {
	r->a_buf *= 2;
	r->buf = fatal_realloc(r->buf, r->a_buf);
    }

    if ((n_bytes = recv(r->fd, &r->buf[r->n_buf], r->a_buf - r->n_buf, 0)) <= 0) {
	return NULL;
    }

    while (n_bytes-- > 0) {
	if (r->buf[r->n_buf] == '\n' || r->buf[r->n_buf] == '\r') {
	    if (r->n_buf > 0) {
		r->buf[r->n_buf] = '\0';
		return r->buf;
	    }
	    memmove(r->buf, &r->buf[r->n_buf + 1], n_bytes);
	    r->n_buf = 0;
	} else {
	    r->n_buf++;
	}
    }

    return NULL;
}

void
net_line_reader_destroy(net_line_reader_t *r)
{
    free(r->buf);
    free(r);
}
