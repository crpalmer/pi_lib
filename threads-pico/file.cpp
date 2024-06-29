#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "pi.h"

#include "ff_headers.h"
#include "ff_sddisk.h"
#include "ff_stdio.h"
#include "ff_utils.h"
#include "hw_config.h"

#include "pi-threads.h"
#include "consoles.h"
#include "string-utils.h"

#include "file.h"

#define C_DECL extern "C"

static PiMutex *init_lock;
static bool is_init = false;
static PiMutex *ff_lock;

static inline void lock() { if (ff_lock) ff_lock->lock(); }
static inline void unlock() { if (ff_lock) ff_lock->unlock(); }

static const char *root = "/sd0";

static const char *filename_to_sd_filename(const char *fname) {
    if (fname[0] != '/') return fname;

    char *full_fname = (char *) fatal_malloc(strlen(root) + strlen(fname) + 1);
    sprintf(full_fname, "%s%s", root, fname);
    return full_fname;
}

static void full_fname_free(const char *fname, const char *full_fname) {
    if (fname != full_fname) fatal_free((void *) full_fname);
}

static void init_once() {
    if (is_init) return;
    init_lock->lock();
    ff_lock = new PiMutex();
    if (! is_init) {
	is_init = true;

	FF_Disk_t *pxDisk = FF_SDDiskInit("sd0");
	assert(pxDisk);
	FF_Error_t xError = FF_SDDiskMount(pxDisk);
	if (FF_isERR(xError) != pdFALSE) {
	    consoles_fatal_printf("FF_SDDiskMount: %s\n", (const char *)FF_GetErrMessage(xError));
	}
	FF_FS_Add(root, pxDisk);
	uint64_t mb;
	unsigned pct;

	getFree(pxDisk, &mb, &pct);
	consoles_printf("%s: mount - %u free MB (%u %%)\n", root, (unsigned) mb, pct);
	ff_chdir(root);
    }
    init_lock->unlock();
}

C_DECL void file_init(void) {
    init_lock = new PiMutex();
}

off_t file_size(const char *fname) {
    FF_Stat_t stat;
    off_t size;
    const char *full_fname = filename_to_sd_filename(fname);

    lock();
    if (ff_stat(full_fname, &stat) >= 0) size = stat.st_size;
    else size = -1;
    unlock();

    full_fname_free(fname, full_fname);
    return size;
}

bool
file_exists(const char *fname)
{
    FF_Stat_t stat;
    const char *full_fname = filename_to_sd_filename(fname);

    lock();
    bool ret = ff_stat(full_fname, &stat) >= 0;
    unlock();

    full_fname_free(fname, full_fname);
    return ret;
}

C_DECL file_t *file_open(const char *fname, const char *mode) {
    init_once();

    const char *full_fname = filename_to_sd_filename(fname);

    lock();
    file_t *file = ff_fopen(full_fname, mode);
    unlock();

    full_fname_free(fname, full_fname);

    return file;
}

C_DECL void file_printf(file_t *file, const char *fmt, ...) {
    assert(is_init);

    va_list va;

    va_start(va, fmt);
    size_t need = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    char *buf = (char *) fatal_malloc(need + 1);

    va_start(va, fmt);
    vsnprintf(buf, need+1, fmt, va);
    va_end(va);

    file_write(file, buf, need);
    fatal_free(buf);
}

#if 0
C_DECL int file_scanf(file_t *file, const char *fmt, ...) {
   assert(is_init);
   va_list va;
   va_start(va, fmt);
   lock();
   int ret = ff_vfscanf(file, fmt, va);
   unlock();
   va_end(va);

   return 0;
}
#endif

C_DECL bool file_gets(file_t *file, char *buf, int n_buf) {
    lock();
    bool ret = ff_fgets(buf, n_buf, (FF_FILE *) file) != NULL;
    unlock();
    return ret;
}

C_DECL size_t file_write(file_t *file, void *data, size_t n_data) {
   assert(is_init);
   lock();
   size_t ret = ff_fwrite(data, 1, n_data, (FF_FILE *) file);
   unlock();
   return ret;
}

C_DECL size_t file_read(file_t *file, void *data, size_t n_data) {
   assert(is_init);
   lock();
   size_t ret = ff_fread(data, 1, n_data, (FF_FILE *) file);
   unlock();
   return ret;
}


C_DECL bool file_seek_abs(file_t *file, unsigned long pos) {
   assert(is_init);
   lock();
   size_t ret = ff_fseek((FF_FILE *) file, (long) pos, SEEK_SET);
   unlock();
   return ret;
}

C_DECL bool file_seek_rel(file_t *file, long delta) {
   assert(is_init);
   lock();
   bool ret = ff_fseek((FF_FILE *) file, (long) delta, SEEK_CUR);
   unlock();
   return ret;
}

C_DECL void file_close(file_t *file) {
   assert(is_init);
   lock();
   ff_fclose((FF_FILE *) file);
   unlock();
}

C_DECL bool file_is_eof(file_t *file) {
   assert(is_init);
   lock();
   bool ret = ff_feof((FF_FILE *) file);
   unlock();
   return ret;
}
