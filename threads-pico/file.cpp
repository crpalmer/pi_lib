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

static char *filename_to_sd_filename(const char *fname) {
    char *full_fname;

    if (fname[0] == '/') full_fname = maprintf("/sd0%s", fname);
    else full_fname = strdup(fname);

    int j = 1;
    for (int i = 1; full_fname[i]; i++) {
	if (full_fname[j-1] == '/' && full_fname[i] == '/') {
	    // We have // skip the second slash
	} else {
	    full_fname[j++] = full_fname[i];
	}
    }
    full_fname[j] = '\0';

    return full_fname;
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
	FF_FS_Add("/sd0", pxDisk);
    }
    init_lock->unlock();
}

C_DECL void file_init(void) {
    init_lock = new PiMutex();
}

off_t file_size(const char *fname) {
    FF_Stat_t stat;
    off_t size;
    char *full_fname = filename_to_sd_filename(fname);

    lock();
    if (ff_stat(full_fname, &stat) >= 0) size = stat.st_size;
    else size = -1;
    unlock();

    fatal_free(full_fname);
    return size;
}

bool
file_exists(const char *fname)
{
    FF_Stat_t stat;

    lock();
    bool ret = ff_stat(fname, &stat) >= 0;
    unlock();
    return ret;
}

C_DECL file_t *file_open(const char *fname, const char *mode) {
    init_once();

    char *full_fname = filename_to_sd_filename(fname);
    lock();
    file_t *file = ff_fopen(full_fname, mode);
    unlock();
    fatal_free(full_fname);

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
