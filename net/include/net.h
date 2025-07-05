/** @defgroup Net Network related tools
 * @{
 */

#ifndef __NET_H__
#define __NET_H__

#ifdef PLATFORM_pico
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PLATFORM_pico
#include <unistd.h>
static inline void closesocket(int socket) { close(socket); }
#endif

int net_connect_tcp(const char *hostname, uint16_t port);
int net_connect_udp(const char *hostname, uint16_t port);
#define net_connect(hostname, port) net_connect_tcp(hostname, port)

int net_listen(uint16_t port);

int net_get_sntp_time(const char *host, struct timespec *now);

void net_platform_init();

#ifdef __cplusplus
};
#endif

/** @} */

#endif
