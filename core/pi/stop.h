#ifndef __STOP_H__
#define __STOP_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stopS stop_t;

stop_t *stop_new(void);

void stop_request_stop(stop_t *);
void stop_await_stop(stop_t *);
void stop_stop(stop_t *);

bool stop_requested(stop_t *);
void stop_stopped(stop_t *);

bool stop_is_stopped(stop_t *);

void stop_reset(stop_t *);

void stop_destroy(stop_t *);

#ifdef __cplusplus
};
#endif

#endif
