#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PLATFORM_pico

void pi_init_with_wifi(void (*main)(void *), void *args);
bool wifi_is_connected();
void wifi_wait_for_connection();

#else

static inline void pi_init_with_wifi(void (*main)(void *), void *args) {
    pi_init();
    main(args);
}

static inline bool wifi_is_connected() { return true; }
static inline void wifi_wait_for_connection() {}

#endif

#ifdef __cplusplus
};
#endif

#endif
