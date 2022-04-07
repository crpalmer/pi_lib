#ifndef __CALL_EVERY_MS_H__
#define __CALL_EVERY_MS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PI_PICO
#include <pico/time.h>
#endif

typedef struct call_everyS call_every_t;
typedef void (*call_every_func_t)(void *data);

call_every_t *
call_every_new(unsigned ms, call_every_func_t func, void *data);

void
call_every_start(call_every_t *e);

void
call_every_stop(call_every_t *e);

void
call_every_destroy(call_every_t *e);

#ifdef __cplusplus
};
#endif

#endif

