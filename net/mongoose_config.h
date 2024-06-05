#ifndef __MONGOOSE_CONFIG_H__
#define __MONGOOSE_CONFIG_H__

#include <alloca.h>
#undef MG_ARCH
#ifdef PLATFORM_pico
#define MG_ARCH MG_ARCH_RP2040
#include "net.h"
#include <fcntl.h>
#else
#define MG_ARCH MG_ARCH_UNIX
#endif

/* Expose some internal functions we want */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mg_str (*guess_content_type_fn_t)(struct mg_str path, const char *extra);

#ifdef NOT_IN_MONGOOSE_C
extern guess_content_type_fn_t ptr_to_guess_content_type;
#else
static struct mg_str guess_content_type(struct mg_str path, const char *extra);
guess_content_type_fn_t ptr_to_guess_content_type = (void *) guess_content_type;
#endif

#ifdef __cplusplus
};
#endif

#endif
