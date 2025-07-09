#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PLATFORM_pico

void wifi_init(const char *hostname);
bool wifi_is_connected();
void wifi_wait_for_connection();

#else

#include "net.h"

static inline void wifi_init(const char *hostname) { net_platform_init(); }
static inline bool wifi_is_connected() { return true; }
static inline void wifi_wait_for_connection() {}

#endif

#ifdef __cplusplus
};
#endif

#endif
