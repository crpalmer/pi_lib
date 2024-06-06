#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#define NO_SYS                      0

/* Include / setup options */

#define LWIP_POSIX_SOCKETS_IO_NAMES 0 /* uses #define for common words like "read", "write"!!! */
#define LWIP_TIMEVAL_PRIVATE 0
#undef LWIP_PROVIDE_ERRNO
#include <errno.h>

/* Library modules enabled */

#define LWIP_NETIF_LOOPBACK       1
#define LWIP_NETIF_HOSTNAME       1
#define LWIP_NETCONN              1

#define LWIP_DHCP                 1
#define LWIP_DNS                  1
#define TCP_LISTEN_BACKLOG        1

/* API Features */

#define SO_REUSE                  1
#define LWIP_SO_SNDTIMEO          1
#define LWIP_SO_RCVTIMEO          1
#define LWIP_TCP_KEEPALIVE        1

/* MBox sizes: defaults are all 0 which crash freertos */

#define DEFAULT_ACCEPTMBOX_SIZE   8
#define DEFAULT_RAW_RECVMBOX_SIZE 8
#define DEFAULT_UDP_RECVMBOX_SIZE 8
#define DEFAULT_TCP_RECVMBOX_SIZE 8
#define TCPIP_MBOX_SIZE 8

/* Threading options */

#define LWIP_TCPIP_CORE_LOCKING_INPUT 1
// TODO: Figure this out #define LWIP_NETCONN_SEM_PER_THREAD 1

#define TCPIP_THREAD_STACKSIZE        1024
#define DEFAULT_THREAD_STACKSIZE      1024

/* Memory */

#define MEM_LIBC_MALLOC               1
#define MEM_ALIGNMENT                 4

/* Buffer sizes */

#define MEMP_NUM_NETCONN	      32
#define MEMP_NUM_TCP_PCB	      32
#define MEMP_NUM_TCP_SEG	      (MEMP_NUM_TCP_PCB*2)
#define MEMP_NUM_UDP_PCB	      8
#define TCP_MSS                       1460
#define TCP_WND                       (8 * TCP_MSS)
#define TCP_SND_BUF                   (8 * TCP_MSS)
#define TCP_SND_QUEUELEN              ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

/* Debugging */

#ifndef NDEBUG
#define LWIP_DEBUG                1
#define LWIP_STATS                1
#define LWIP_STATS_DISPLAY        1
#endif

#define ETHARP_DEBUG              LWIP_DBG_OFF
#define NETIF_DEBUG               LWIP_DBG_OFF
#define PBUF_DEBUG                LWIP_DBG_OFF
#define API_LIB_DEBUG             LWIP_DBG_OFF
#define API_MSG_DEBUG             LWIP_DBG_OFF
#define SOCKETS_DEBUG             LWIP_DBG_OFF
#define ICMP_DEBUG                LWIP_DBG_OFF
#define INET_DEBUG                LWIP_DBG_OFF
#define IP_DEBUG                  LWIP_DBG_OFF
#define IP_REASS_DEBUG            LWIP_DBG_OFF
#define RAW_DEBUG                 LWIP_DBG_OFF
#define MEM_DEBUG                 LWIP_DBG_OFF
#define MEMP_DEBUG                LWIP_DBG_OFF
#define SYS_DEBUG                 LWIP_DBG_OFF
#define TCP_DEBUG                 LWIP_DBG_OFF
#define TCP_INPUT_DEBUG           LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG          LWIP_DBG_OFF
#define TCP_RTO_DEBUG             LWIP_DBG_OFF
#define TCP_CWND_DEBUG            LWIP_DBG_OFF
#define TCP_WND_DEBUG             LWIP_DBG_OFF
#define TCP_FR_DEBUG              LWIP_DBG_OFF
#define TCP_QLEN_DEBUG            LWIP_DBG_OFF
#define TCP_RST_DEBUG             LWIP_DBG_OFF
#define UDP_DEBUG                 LWIP_DBG_OFF
#define TCPIP_DEBUG               LWIP_DBG_OFF
#define PPP_DEBUG                 LWIP_DBG_OFF
#define SLIP_DEBUG                LWIP_DBG_OFF
#define DHCP_DEBUG                LWIP_DBG_OFF
#define HTTPD_DEBUG               LWIP_DBG_OFF
#define HTTPD_DEBUG_TIMING        LWIP_DBG_OFF

#endif /* __LWIPOPTS_H__ */
