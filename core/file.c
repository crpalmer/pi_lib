#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "pi.h"
#include "mem.h"
#include "string-utils.h"

#ifndef PLATFORM_pico
static const char *media_dirs[] = {
     "/home/crpalmer/halloween-media",
     "/home/crpalmer/halloween-media.master"
};

#define N_MEDIA_DIRS (sizeof(media_dirs) / sizeof(media_dirs[0]))
#endif

file_t *
media_file_open_read(const char *fname)
{
    file_t *f = file_open(fname, "rb");

#ifndef PLATFORM_pico
    for (int i = 0; i < N_MEDIA_DIRS && ! f; i++) {
	char *this_name;
	this_name = maprintf("%s/%s", media_dirs[i], fname);
        f = file_open(this_name, "rb");
        fatal_free(this_name);
    }
#endif

    return f;
}
