#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include "mem.h"
#include "net-line-reader.h"
#include "global-trace.h"

#define MIN_READ_SIZE	1024

struct net_line_readerS {
    int   fd;
    char *buf;
    int   a_buf, n_buf;
    net_line_reader_callback_t cb;
    void *data;
};

net_line_reader_t *
net_line_reader_new(int fd, net_line_reader_callback_t cb, void *data)
{
    net_line_reader_t *r = fatal_malloc(sizeof(*r));

    r->fd = fd;
    r->a_buf = 1024;
    r->n_buf = 0;
    r->buf = malloc(r->a_buf);
    r->cb = cb;
    r->data = data;

    return r;
}

int
net_line_reader_read(net_line_reader_t *r)
{
    int n_bytes;

    if (r->a_buf - r->n_buf < MIN_READ_SIZE) {
	r->a_buf *= 2;
	r->buf = fatal_realloc(r->buf, r->a_buf);
    }

    n_bytes = read(r->fd, &r->buf[r->n_buf], r->a_buf - r->n_buf);

    if (n_bytes < 0) return n_bytes;

    while (n_bytes-- > 0) {
	if (r->buf[r->n_buf] == '\n' || r->buf[r->n_buf] == '\r') {
	    if (r->n_buf > 0) {
		r->buf[r->n_buf] = '\0';
		if (global_trace) fprintf(stderr, "%s: %d line %s\n", __func__, r->fd, r->buf);
		r->cb(r->data, r->fd, r->buf);
	    }
	    memmove(r->buf, &r->buf[r->n_buf + 1], n_bytes);
	    r->n_buf = 0;
	} else {
	    r->n_buf++;
	}
    }

    return 0;
}

void
net_line_reader_destroy(net_line_reader_t *r)
{
    free(r->buf);
    free(r);
}
