#ifndef __GLOBAL_TRACE__
#define __GLOBAL_TRACE__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

extern bool global_trace;

#define ENABLE_GLOBAL_TRACE() (global_trace = 1)

#ifdef __cplusplus
};
#endif

#endif
