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

const char *external_cyw43_host_name = "external-host-name";

static void connect_to_wifi(void *unused)
{
    printf("WiFi: trying to connect.\n");
    while (1) {
        int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        if (link_status != CYW43_LINK_UP) {
	    is_connected = false;

            printf("WiFi: Connection status: %d\n", link_status);
            if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
                printf("failed to connect.\n");
            } else {
		printf("WiFi: Connected.\n");
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

void
wifi_init(const char *hostname)
{
    external_cyw43_host_name = hostname;

    m_connection = pi_mutex_new();
    c_connection = pi_cond_new();

    net_platform_init();

    cyw43_arch_enable_sta_mode();
    cyw43_wifi_pm(&cyw43_state, cyw43_pm_value(CYW43_NO_POWERSAVE_MODE, 20, 1, 1, 1));

    pi_thread_create("wifi up", connect_to_wifi, NULL);
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
