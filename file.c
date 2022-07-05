#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "global-trace.h"
#include "string-utils.h"

#include "file.h"

int
echo(const char *fname, const char *fmt, ...)
{
    FILE *f;
    va_list va;
    int ret;

    if ((f = fopen(fname, "w")) == NULL) {
	return -1;
    }

    va_start(va, fmt);
    ret = vfprintf(f, fmt, va);
    va_end(va);

    fclose(f);

    return ret;
}

void
fatal_echo(const char *fname, const char *fmt, ...)
{
    FILE *f;
    va_list va;

    if ((f = fopen(fname, "w")) == NULL) {
	fprintf(stderr, "failed to open [%s]\n", fname);
	exit(1);
    }

    if (global_trace) {
	va_start(va, fmt);
	fprintf(stderr, "%s: [%s]: ", __func__, fname);
	vfprintf(stderr, fmt, va);
	va_end(va);
    }

    va_start(va, fmt);
    if (vfprintf(f, fmt, va) < 0) {
	fprintf(stderr, "failed to write to [%s]\n", fname);
	exit(1);
    }
    va_end(va);

    fclose(f);
}

static const char *media_dirs[] = {
     ".",
     "/home/crpalmer/halloween-media",
     "/home/crpalmer/halloween-media.master"
};

#define N_MEDIA_DIRS (sizeof(media_dirs) / sizeof(media_dirs[0]))

FILE *
media_fopen_read(const char *fname)
{
    FILE *f = NULL;

    for (int i = 0; i < N_MEDIA_DIRS && ! f; i++) {
        char *this_name = maprintf("%s/%s", media_dirs[i], fname);

        f = fopen(this_name, "rb");
        free(this_name);
    }

    return f;
}
