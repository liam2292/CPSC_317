/*
 * Version 1.0
 */


#ifndef __STCP_H_
#define __STCP_H_

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "tcp.h"
#include "log.h"

#define STCP_MAXWIN    65535 
#define STCP_MTU       300     /* MTU size */
#define STCP_MSS       (STCP_MTU - sizeof(tcpheader)) /* MSS Size */
#define STCP_READ_TIMED_OUT (-3)
#define STCP_READ_PERMANENT_FAILURE (-4)
#define STCP_INITIAL_TIMEOUT 1000
#define STCP_MIN_TIMEOUT 1000
#define STCP_MAX_TIMEOUT 4000
#define STCP_INFINITE_TIMEOUT 10000
#define STCP_TIME_WAIT_DURATION 2000
#define EXCESS_FIN_THRESHOLD 3

static inline int min(int a, int b) { return a < b ? a : b; }
static inline int max(int a, int b) { return a > b ? a : b; }

static inline int stcpNextTimeout(int timeout) { return min(STCP_MAX_TIMEOUT, timeout * 2); }

/* In the above if MSS and MTU don't mean anything to you then read the text */

/* The sender can be in five possible states: CLOSED, SYN_SENT, ESTABLISHED, CLOSING and FIN_WAIT */
#define STCP_SENDER_CLOSED 0
#define STCP_SENDER_SYN_SENT 1
#define STCP_SENDER_ESTABLISHED 2
#define STCP_SENDER_CLOSING 3
#define STCP_SENDER_FIN_WAIT 4

typedef struct packet {
    unsigned char data[STCP_MTU];
    tcpheader *hdr;
    int len;
} packet;

static inline int payloadSize(packet *pkt) {
    return pkt->len - sizeof(tcpheader);
}

static inline void initPacket(packet *pkt, unsigned char *data, int len) {
    pkt->len = len;
    pkt->hdr = (tcpheader *)pkt->data;
    if (data != NULL) memcpy(pkt->data, data, len);
}

/* Declarations for STCP.C */

extern void createSegment(packet *pkt, int flags, unsigned short rwnd, unsigned int seq, unsigned int ack, unsigned char *data, int len);
extern void dump(char dir, void* pkt, int len);
extern unsigned int hostname_to_ipaddr(const char *s);
extern int readWithTimeout(int fd, unsigned char *pkt, int ms);
extern unsigned short ipchecksum(void *data, int len);
extern int udp_open(char *remote_IP_str, int remote_port, int local_port);

#include "wraparound.h"

#endif
