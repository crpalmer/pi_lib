#ifndef __FILE_H__
#define __FILE_H__

#ifdef __cplusplus
extern "C" {
#endif

int
echo(const char *fname, const char *fmt, ...);

void
fatal_echo(const char *fname, const char *fmt, ...);

FILE *
media_fopen_read(const char *fname);

#ifdef __cplusplus
};
#endif

#endif
