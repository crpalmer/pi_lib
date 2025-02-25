#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "pi.h"
#include "pico-sd-cards.h"

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

const bool trace_open_close = false;

static PiMutex *init_lock;
static bool is_init = false;
static PiMutex *ff_lock;

static inline void lock() { if (ff_lock) ff_lock->lock(); }
static inline void unlock() { if (ff_lock) ff_lock->unlock(); }

static const char *cwd;

static char *fix_fname(const char *fname, const char *fname2 = NULL) {
    char *fixed_fname;

    if (fname2) {
	fixed_fname = (char *) fatal_malloc(strlen(fname) + 1 + strlen(fname2) + 1);
	sprintf(fixed_fname, "%s/%s", fname, fname2);
    } else {
	fixed_fname = fatal_strdup(fname);
    }

    assert(fixed_fname[0]);

    int i = 1, j = 1;
    while (fixed_fname[i]) {
	if (fixed_fname[i] != '/' || fixed_fname[j-1] != '/') fixed_fname[j++] = fixed_fname[i];
	i++;
    }
    fixed_fname[j] = '\0';

    return fixed_fname;
}

static void init_once() {
    if (is_init) return;
    init_lock->lock();
    ff_lock = new PiMutex();
    if (! is_init) {
	is_init = true;

	if (pico_get_n_sd_cards() == 0) pico_add_pico_board_sd_card("/sd0");

	bool first_mount = true;

	for (int i = 0; i < pico_get_n_sd_cards(); i++) {
	    const char *root = pico_get_sd_card_root(i);
	    FF_Disk_t *pxDisk = FF_SDDiskInit(root);
	    if (! pxDisk) {
		consoles_printf("Failed to initialize disk: %s\n", root);
		continue;
	    }
	    FF_Error_t xError = FF_SDDiskMount(pxDisk);
	    if (FF_isERR(xError) != pdFALSE) {
		consoles_printf("Failed to mount: %s error %s\n", root, (const char *)FF_GetErrMessage(xError));
		continue;
	    }
	    FF_FS_Add(root, pxDisk);
	    uint64_t mb;
	    unsigned pct;

	    getFree(pxDisk, &mb, &pct);
	    consoles_printf("%s: mount - %u free MB (%u %%)\n", root, (unsigned) mb, pct);
	    if (first_mount) {
		cwd = root;
		ff_chdir(cwd);
		first_mount = false;
	    }
	}
    }
    init_lock->unlock();
}

C_DECL void file_init(void) {
    init_lock = new PiMutex();
}

off_t file_size(const char *fname) {
    init_once();

    char *fixed_fname = fix_fname(fname);

    lock();
    FF_Stat_t stat;
    off_t size;
    if (ff_stat(fname, &stat) >= 0) size = stat.st_size;
    else size = -1;
    unlock();

    fatal_free(fixed_fname);

    return size;
}

bool
file_exists(const char *fname)
{
    init_once();

    char *fixed_fname = fix_fname(fname);
    
    lock();
    FF_Stat_t stat;
    bool exists = ff_stat(fixed_fname, &stat) >= 0;
    unlock();

    fatal_free(fixed_fname);

    return exists;
}

C_DECL file_t *file_open(const char *fname, const char *mode) {
    init_once();

    char *fixed_fname = fix_fname(fname);

    lock();
    file_t *file = ff_fopen(fixed_fname, mode);
    unlock();

    if (file && trace_open_close) printf("%s: %s -> %p\n", __func__, fixed_fname, file);

    fatal_free(fixed_fname);

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
    if (trace_open_close) printf("%s: %p\n", __func__, file);
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

bool FileForeach::foreach(const char *dir) {
    init_once();

    if (dir == NULL || dir[0] == '\0') dir = cwd;

    if (strcmp(dir, "/") == 0) {
	for (int i = 0; i < pico_get_n_sd_cards(); i++) {
	    directory(pico_get_sd_card_root(i));
	}
	return true;
    }

    char *fixed_dir = fix_fname(dir);

    FF_FindData_t data;

    int ret = ff_findfirst(fixed_dir, &data);
    fatal_free(fixed_dir);
    if (ret != 0) return false;

    do {
	if ((data.ucAttributes & (FF_FAT_ATTR_HIDDEN | FF_FAT_ATTR_SYSTEM)) != 0) continue;
	if (strcmp(data.pcFileName, ".") == 0 || strcmp(data.pcFileName, "..") == 0) continue;
	char *fixed_fname = fix_fname(dir, data.pcFileName);
	if ((data.ucAttributes & FF_FAT_ATTR_DIR) != 0) directory(fixed_fname);
	else file(fixed_fname);
	fatal_free(fixed_fname);
    } while (ff_findnext(&data) == 0);

    return true;
}
