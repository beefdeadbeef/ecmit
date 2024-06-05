/* -*- mode: c; tab-width: 8 -*-
 */

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

#define NO_SYS				0
#define LWIP_TCPIP_CORE_LOCKING		1
#define LWIP_TCPIP_CORE_LOCKING_INPUT	1

#define LWIP_SOCKET             !NO_SYS
#define LWIP_NETCONN            !NO_SYS
#define LWIP_NETIF_API          !NO_SYS

#define LWIP_SINGLE_NETIF       1
#define LWIP_ARP                1
#define LWIP_DHCP               1

#define LWIP_NETIF_LINK_CALLBACK	1
#define LWIP_NETIF_STATUS_CALLBACK	1
/*
 */
#define MEM_ALIGNMENT           4
#define MEM_USE_POOLS           1
#define MEMP_USE_CUSTOM_POOLS   1
/*
 */
#define DEFAULT_THREAD_STACKSIZE	BGRT_PROC_STACK_SIZE
#define TCPIP_THREAD_STACKSIZE		(2 * BGRT_PROC_STACK_SIZE)
#define MAX_THREAD_STACKSIZE		(3 * BGRT_PROC_STACK_SIZE)
/*
 */
#define DEFAULT_THREAD_PRIO	BGRT_PRIO_LOWEST
#define TCPIP_THREAD_PRIO	BGRT_PRIO_LOWEST
/*
 */
#define LWIP_DBG_MIN_LEVEL      LWIP_DBG_LEVEL_ALL

#define ECM_DEBUG		LWIP_DBG_ON
#define SYS_ARCH_DEBUG		LWIP_DBG_ON
#define NETIF_DEBUG             LWIP_DBG_OFF
#define ETHARP_DEBUG            LWIP_DBG_OFF
#define IP_DEBUG                LWIP_DBG_OFF
#define IP_REASS_DEBUG          LWIP_DBG_OFF
#define INET_DEBUG              LWIP_DBG_OFF
#define TCPIP_DEBUG             LWIP_DBG_OFF
#define ICMP_DEBUG              LWIP_DBG_OFF
#define TCP_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG               LWIP_DBG_OFF
#define MEMP_DEBUG              LWIP_DBG_ON
#define MEM_DEBUG               LWIP_DBG_ON
#define PBUF_DEBUG              LWIP_DBG_OFF

#define LWIP_DBG_TYPES_ON	(LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT)

#endif // LWIPOPTS_H_
