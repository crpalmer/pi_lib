#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "ff_headers.h"
#include "ff_sddisk.h"
#include "ff_stdio.h"
#include "ff_utils.h"
#include "hw_config.h"

#include "pi.h"
#include "pi-threads.h"
#include "consoles.h"
#include "string-utils.h"

#include "file.h"

#define C_DECL extern "C"

static PiMutex *init_lock;
static bool is_init = false;

static void init_once() {
    if (is_init) return;
    init_lock->lock();
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

C_DECL file_t *file_open(const char *fname, const char *mode) {
    init_once();

    char *full_fname = maprintf("/sd0/%s", fname);
    file_t *file = ff_fopen(full_fname, mode);
    free(full_fname);

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
    free(buf);
}

#if 0
C_DECL int file_scanf(file_t *file, const char *fmt, ...) {
   assert(is_init);
   va_list va;
   va_start(va, fmt);
   int ret = ff_vfscanf(file, fmt, va);
   va_end(va);

   return 0;
}
#endif

C_DECL size_t file_write(file_t *file, void *data, size_t n_data) {
   assert(is_init);
   return ff_fwrite(data, 1, n_data, (FF_FILE *) file);
}

C_DECL size_t file_read(file_t *file, void *data, size_t n_data) {
   assert(is_init);
   return ff_fread(data, 1, n_data, (FF_FILE *) file);
}


C_DECL bool file_seek_abs(file_t *file, unsigned long pos) {
   assert(is_init);
   return ff_fseek((FF_FILE *) file, (long) pos, SEEK_SET);
}

C_DECL bool file_seek_rel(file_t *file, long delta) {
   assert(is_init);
   return ff_fseek((FF_FILE *) file, (long) delta, SEEK_CUR);
}

C_DECL void file_close(file_t *file) {
   assert(is_init);
   ff_fclose((FF_FILE *) file);
}

C_DECL bool file_is_eof(file_t *file) {
   assert(is_init);
   return ff_feof((FF_FILE *) file);
}