/*
 * Version 1.0
 */


#ifndef __UTIL_H__
#define __UTIL_H__

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "ip.h"
#include "log.h"

#define UTIL_MTU       300 /* MTU size */
#define UTIL_MSS       (UTIL_MTU - sizeof(ipheader)) /* MSS Size */
#define UTIL_READ_PERMANENT_FAILURE (-4)

static inline int payloadSize(packet *pkt) {
    return pkt->len - sizeof(ipheader);
}

static inline void initPacket(packet *pkt, unsigned char *data, int len) {
    pkt->len = len;
    pkt->hdr = (ipheader *)pkt->data;
    if (data != NULL) memcpy(pkt->data, data, len);
}

static inline packet *makePacket(char *data, int len) {
    packet *ans = (packet *)malloc(sizeof(*ans));
    ans->len = len + sizeof(ipheader);
    ans->hdr = (ipheader *)ans->data;
    memcpy(&ans->data[sizeof(ipheader)], data, len);
    return ans;
}

/* Declarations for util.c */

extern void dump(char dir, int interface, void* pkt, int len);
extern unsigned int hostname_to_ipaddr(const char *s);
extern int sendpkt(int fd, int interface, packet *pkt);
extern int readpkt(int fd, packet *pkt, int *interface);
extern int udp_open(int local_port);
extern int getDefaultPort();
#endif
