#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PLATFORM_pico

void wifi_init();
bool wifi_is_connected();
void wifi_wait_for_connection();

#else

static inline void wifi_init() { }
static inline bool wifi_is_connected() { return true; }
static inline void wifi_wait_for_connection() {}

#endif

#ifdef __cplusplus
};
#endif

#endif
