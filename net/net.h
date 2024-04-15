#ifndef __NET_H__
#define __NET_H__

#ifdef PLATFORM_pico
#include "lwip/ip4_addr.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int net_listen(uint16_t port);

#ifdef __cplusplus
};
#endif

#endif
