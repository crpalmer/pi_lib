#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "mem.h"
#include "net.h"
#include "pi-threads.h"
#include "time-utils.h"
#include "wifi.h"

static bool is_connected = false;
static pi_mutex_t *m_connection;
static pi_cond_t *c_connection;

static const char *ssid = WIFI_SSID;
static const char *password = WIFI_PASSWORD;

static void connect_to_wifi(void *unused)
{
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        exit(1);
    }

    cyw43_arch_enable_sta_mode();
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

    printf("trying to connected to %s %s\n", ssid, password);
    while (1) {
        int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        if (link_status != CYW43_LINK_UP) {
	    is_connected = false;

            printf("Connecting to Wi-Fi, current status is %d\n", link_status);
            if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
                printf("failed to connect.\n");
            } else {
printf("Connected.\n");
		pi_mutex_lock(m_connection);
		is_connected = true;
		pi_cond_broadcast(c_connection);
		pi_mutex_unlock(m_connection);
            }
        } else {
	    ms_sleep(5*1000);
        }
    }
}

typedef struct {
    void (*main)(void *);
    void *args;
} start_args_t;

static void
run_main(void *start_args_as_vp)
{
    start_args_t *start_args = (start_args_t *) start_args_as_vp;

    wifi_wait_for_connection();
    pi_thread_create("main", start_args->main, start_args->args);
    free(start_args);
}

void
pi_init_with_wifi(void (*main)(void *), void *args)
{
    pi_init_with_threads();

    m_connection = pi_mutex_new();
    c_connection = pi_cond_new();

    pi_thread_create("wifi up", connect_to_wifi, NULL);

    start_args_t *start_args = fatal_malloc(sizeof(*start_args));
    start_args->main = main;
    start_args->args = args;
    pi_thread_create("run-main", run_main, start_args);

    pi_threads_start_and_wait();
}

bool
wifi_is_connected()
{
    return is_connected;
}

void
wifi_wait_for_connection()
{
    pi_mutex_lock(m_connection);
    while (! is_connected) {
	pi_cond_wait(c_connection, m_connection);
    }
    pi_mutex_unlock(m_connection);
}
