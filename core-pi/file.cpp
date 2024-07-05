#include <dirent.h>
#include <stdarg.h>
#include <string.h>
#include "pi.h"
#include "string-utils.h"
#include "file.h"

off_t file_size(const char *fname) {
    struct stat statbuf;

    if (stat(fname, &statbuf) < 0) return -1;
    return statbuf.st_size;
}

bool
file_exists(const char *fname)
{
    struct stat s;

    return stat(fname, &s) >= 0;
}

file_t *file_open(const char *fname, const char *mode) { return fopen(fname, mode); }

void file_printf(file_t *file, const char *fmt, ...) {
   va_list va;
   va_start(va, fmt);
   vfprintf(file, fmt, va);
   va_end(va);
}

int file_scanf(file_t *file, const char *fmt, ...) {
   va_list va;
   va_start(va, fmt);
   int ret = vfscanf(file, fmt, va);
   va_end(va);

   return ret;
}

bool file_gets(file_t *file, char *buf, int n_buf) {
    return fgets(buf, n_buf, file) != NULL;
}

size_t file_write(file_t *file, void *data, size_t n_data) { return fwrite(data, 1, n_data, file); }
size_t file_read(file_t *file, void *data, size_t n_data) { return fread(data, 1, n_data, file); }

bool file_seek_abs(file_t *file, unsigned long pos) { return fseek(file, (long) pos, SEEK_SET) >= 0; }
bool file_seek_rel(file_t *file, long delta) { return fseek(file, delta, SEEK_CUR) >= 0; }

void file_close(file_t *file) { fclose(file); }

bool file_is_eof(file_t *file) { return feof(file) != 0; }

bool FileForeach::foreach(const char *dir) {
    DIR *d;
    struct dirent *dirent;

    if ((d = opendir(dir)) == NULL) return false;
    while ((dirent = readdir(d)) != NULL) {
	if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) continue;
	char *fullname = maprintf("%s/%s", dir, dirent->d_name);
	if (dirent->d_type == DT_DIR) directory(fullname);
	else if (dirent->d_type == DT_LNK) symbolic_link(fullname);
	else if (dirent->d_type == DT_REG) file(fullname);
	free(fullname);
    }

    closedir(d);
    return true;
}
